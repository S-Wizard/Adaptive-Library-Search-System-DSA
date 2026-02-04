#include "library_engine.h"
#include <sstream>
#include <map>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <ctime>

/* ================= CONSTRUCTOR / DESTRUCTOR ================= */

LibraryEngine::LibraryEngine()
    : transactionCounter(0), reservationCounter(0) {}

LibraryEngine::~LibraryEngine() {
    for (auto& b : books) delete b.second;
    for (auto& u : users) delete u.second;

    for (auto& q : reservationQueues) {
        while (!q.second.empty()) {
            delete q.second.top();
            q.second.pop();
        }
    }

    while (!transactionHistory.empty()) {
        delete transactionHistory.top();
        transactionHistory.pop();
    }
}

/* ================= BOOKS ================= */

void LibraryEngine::addBook(Book* book) {
    books[book->isbn] = book;
    bookISBNIndex.insert(book->isbn, book);
}

Book* LibraryEngine::getBook(const std::string& isbn) {
    return bookISBNIndex.search(isbn);
}

/* ================= USERS ================= */

void LibraryEngine::addUser(User* user) {
    users[user->userID] = user;
}

User* LibraryEngine::getUser(const std::string& userID) {
    auto it = users.find(userID);
    return (it == users.end()) ? nullptr : it->second;
}

/* ================= SEARCH ================= */

void LibraryEngine::buildSearchIndices() {
    for (auto& p : books) {
        Book* b = p.second;

        std::istringstream ts(b->title), as(b->author);
        std::string word;

        while (ts >> word) titleTrie.insert(word, b->isbn);
        while (as >> word) authorTrie.insert(word, b->isbn);
    }
}

std::vector<SearchResult> LibraryEngine::searchByTitle(const std::string& query) {
    return titleTrie.searchPrefix(query, books);
}

std::vector<SearchResult> LibraryEngine::searchByAuthor(const std::string& query) {
    return authorTrie.searchPrefix(query, books);
}

/* ================= RECOMMENDATIONS ================= */

void LibraryEngine::buildRecommendationGraph() {
    recommendations.buildFromBooks(books);
}

std::vector<SearchResult> LibraryEngine::getRecommendations(
    const std::string& isbn, int limit
) {
    return recommendations.getRecommendations(isbn, limit, books);
}

std::vector<SearchResult> LibraryEngine::getPersonalizedRecommendations(
    const std::string& userID,
    const std::vector<std::string>& recentISBNs,
    int limit
) {
    std::vector<std::string> seeds;
    std::unordered_set<std::string> seedSet;
    std::unordered_set<std::string> excludeSet;

    // Get borrowed + recent activity ISBNs (same logic as getUserProfile)
    std::map<std::string, bool> borrowedBooks;
    std::vector<Transaction*> allTransactions;
    std::stack<Transaction*> tempStack = transactionHistory;
    while (!tempStack.empty()) {
        allTransactions.push_back(tempStack.top());
        tempStack.pop();
    }

    for (auto it = allTransactions.rbegin(); it != allTransactions.rend(); ++it) {
        Transaction* t = *it;
        if (t->userID != userID) continue;

        if (t->type == TransactionType::ISSUE) {
            borrowedBooks[t->bookID] = true;
            excludeSet.insert(t->bookID);
            if (seedSet.insert(t->bookID).second)
                seeds.push_back(t->bookID);
        } else if (t->type == TransactionType::RETURN) {
            if (borrowedBooks.find(t->bookID) != borrowedBooks.end())
                borrowedBooks[t->bookID] = false;
            excludeSet.insert(t->bookID);
            if (seedSet.insert(t->bookID).second)
                seeds.push_back(t->bookID);
        }
    }

    // Add recentISBNs from frontend (searched/viewed books)
    for (const auto& isbn : recentISBNs) {
        if (getBook(isbn) && seedSet.insert(isbn).second)
            seeds.push_back(isbn);
    }

    // Collect recommendations from each seed
    std::unordered_map<std::string, SearchResult> resultMap;
    for (const auto& seed : seeds) {
        auto recs = getRecommendations(seed, limit * 2);
        for (const auto& r : recs) {
            if (excludeSet.count(r.isbn) || seedSet.count(r.isbn)) continue;
            auto it = resultMap.find(r.isbn);
            if (it == resultMap.end() || r.relevanceScore > it->second.relevanceScore)
                resultMap[r.isbn] = r;
        }
    }

    std::vector<SearchResult> results;
    for (auto& p : resultMap) results.push_back(p.second);
    std::sort(results.begin(), results.end(),
        [](const SearchResult& a, const SearchResult& b) {
            return a.relevanceScore > b.relevanceScore;
        });
    if ((int)results.size() > limit) results.resize(limit);
    return results;
}

/* ================= ISSUE / RETURN ================= */

json LibraryEngine::issueBook(const std::string& userID, const std::string& isbn) {
    json res;
    User* user = getUser(userID);
    Book* book = getBook(isbn);

    if (!user || !book) {
        res["success"] = false;
        res["message"] = "Invalid user or book";
        return res;
    }

    if (book->availableCopies == 0) {
        res["success"] = false;
        res["message"] = "No copies available";
        return res;
    }

    book->availableCopies--;

    Transaction* t = new Transaction(
        "TXN_" + std::to_string(transactionCounter++),
        userID,
        isbn,
        "",                     // ✅ FIX: copyID placeholder
        TransactionType::ISSUE
    );

    transactionHistory.push(t);

    titleTrie.updateBorrowImpact(book->title, 1.0);
    authorTrie.updateBorrowImpact(book->author, 1.0);
    book->borrowImpact++;

    res["success"] = true;
    res["message"] = "Book issued successfully";
    res["availableCopies"] = book->availableCopies;
    return res;
}

json LibraryEngine::returnBook(const std::string& userID, const std::string& isbn) {
    json res;
    Book* book = getBook(isbn);

    if (!book || book->availableCopies == book->totalCopies) {
        res["success"] = false;
        res["message"] = "Invalid return";
        return res;
    }

    book->availableCopies++;

    Transaction* t = new Transaction(
        "TXN_" + std::to_string(transactionCounter++),
        userID,
        isbn,
        "",                     // ✅ FIX: copyID placeholder
        TransactionType::RETURN
    );

    transactionHistory.push(t);

    res["success"] = true;
    res["message"] = "Book returned successfully";
    res["availableCopies"] = book->availableCopies;
    return res;
}

/* ================= RESERVATION ================= */

json LibraryEngine::reserveBook(const std::string& userID, const std::string& isbn) {
    json res;
    User* user = getUser(userID);
    Book* book = getBook(isbn);

    if (!user || !book) {
        res["success"] = false;
        res["message"] = "Invalid reservation";
        return res;
    }

    Reservation* r = new Reservation(
        "RES_" + std::to_string(reservationCounter++),
        userID,
        isbn,
        user->type
    );

    reservationQueues[isbn].push(r);

    res["success"] = true;
    res["message"] = "Book reserved successfully";
    res["queuePosition"] = reservationQueues[isbn].size();
    return res;
}

/* ================= UNDO ================= */

json LibraryEngine::undoLastAction() {
    json res;

    if (transactionHistory.empty()) {
        res["success"] = false;
        res["message"] = "Nothing to undo";
        return res;
    }

    Transaction* t = transactionHistory.top();
    transactionHistory.pop();

    Book* book = getBook(t->bookID);
    if (book) {
        if (t->type == TransactionType::ISSUE)
            book->availableCopies++;
        else
            book->availableCopies--;
    }

    delete t;

    res["success"] = true;
    res["message"] = "Undo successful";
    return res;
}

/* ================= PROFILE ================= */

json LibraryEngine::getUserProfile(const std::string& userID) {
    json res;
    User* user = getUser(userID);
    
    if (!user) {
        res["success"] = false;
        res["message"] = "User not found";
        return res;
    }

    // Get user info
    res["user"] = {
        {"userID", user->userID},
        {"name", user->name},
        {"email", user->email},
        {"type", user->type == UserType::FACULTY ? "faculty" : 
                 user->type == UserType::FINAL_YEAR_STUDENT ? "final_year" : "student"},
        {"registrationDate", user->registrationDate}
    };

    // Calculate statistics by analyzing transaction history
    int totalBorrowed = 0;
    int activeBorrows = 0;
    int reservations = 0;
    std::map<std::string, bool> borrowedBooks; // ISBN -> isActive
    
    // Create a copy of transaction history to iterate (stack doesn't support iteration)
    std::vector<Transaction*> allTransactions;
    std::stack<Transaction*> tempStack = transactionHistory;
    while (!tempStack.empty()) {
        allTransactions.push_back(tempStack.top());
        tempStack.pop();
    }

    // Process transactions in reverse chronological order
    for (auto it = allTransactions.rbegin(); it != allTransactions.rend(); ++it) {
        Transaction* t = *it;
        if (t->userID != userID) continue;

        if (t->type == TransactionType::ISSUE) {
            totalBorrowed++;
            borrowedBooks[t->bookID] = true;
            activeBorrows++;
        } else if (t->type == TransactionType::RETURN) {
            if (borrowedBooks.find(t->bookID) != borrowedBooks.end()) {
                borrowedBooks[t->bookID] = false;
                activeBorrows--;
            }
        }
    }

    // Count reservations
    for (auto& queuePair : reservationQueues) {
        std::priority_queue<Reservation*, std::vector<Reservation*>, ReservationCompare> tempQueue = queuePair.second;
        while (!tempQueue.empty()) {
            if (tempQueue.top()->userID == userID) {
                reservations++;
                break;
            }
            tempQueue.pop();
        }
    }

    // Get currently borrowed books with details
    res["borrowedBooks"] = json::array();
    for (auto& pair : borrowedBooks) {
        if (pair.second) { // Only active borrows
            Book* book = getBook(pair.first);
            if (book) {
                // Find the issue transaction for this book
                long long issueDate = 0;
                for (auto it = allTransactions.rbegin(); it != allTransactions.rend(); ++it) {
                    Transaction* t = *it;
                    if (t->userID == userID && t->bookID == pair.first && t->type == TransactionType::ISSUE) {
                        issueDate = t->timestamp;
                        break;
                    }
                }
                
                // Calculate due date (14 days from issue)
                long long dueDate = issueDate + (14 * 24 * 60 * 60);
                long long now = time(nullptr);
                std::string status = "on_time";
                if (now > dueDate) status = "overdue";
                else if ((dueDate - now) < (3 * 24 * 60 * 60)) status = "due_soon";

                res["borrowedBooks"].push_back({
                    {"isbn", book->isbn},
                    {"title", book->title},
                    {"author", book->author},
                    {"category", book->category},
                    {"issueDate", issueDate},
                    {"dueDate", dueDate},
                    {"status", status}
                });
            }
        }
    }

    // Get recent activity (last 10 transactions) - newest first
    std::vector<std::pair<Transaction*, Book*>> userActivities;
    for (auto* t : allTransactions) {
        if (t->userID != userID) continue;
        Book* book = getBook(t->bookID);
        if (book) {
            userActivities.push_back({t, book});
        }
    }
    
    // Sort by timestamp descending (newest first)
    std::sort(userActivities.begin(), userActivities.end(),
        [](const std::pair<Transaction*, Book*>& a, const std::pair<Transaction*, Book*>& b) {
            return a.first->timestamp > b.first->timestamp;
        });
    
    // Take only the 10 most recent
    res["activity"] = json::array();
    int count = 0;
    for (auto& pair : userActivities) {
        if (count >= 10) break;
        Transaction* t = pair.first;
        Book* book = pair.second;
        
        res["activity"].push_back({
            {"type", t->type == TransactionType::ISSUE ? "issue" : "return"},
            {"isbn", book->isbn},
            {"title", book->title},
            {"author", book->author},
            {"timestamp", t->timestamp}
        });
        count++;
    }

    // Statistics
    res["statistics"] = {
        {"totalBorrowed", totalBorrowed},
        {"activeBorrows", activeBorrows},
        {"reservations", reservations},
        {"recommended", 0} // Can be calculated separately if needed
    };

    res["success"] = true;
    return res;
}