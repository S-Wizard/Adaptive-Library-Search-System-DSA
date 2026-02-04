#include "library_engine.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

LibraryEngine* engine = nullptr;

/* ---------------- CSV PARSING ---------------- */

std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

void loadBooksFromCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    bool header = true;

    while (std::getline(file, line)) {
        if (header) {
            header = false;
            continue;
        }

        auto fields = parseCSVLine(line);
        if (fields.size() < 5) continue;

        std::string isbn = fields[0];
        std::string title = fields[1];
        std::string author = fields[2];
        std::string category = fields[3];

        int copies = 1;
        try { copies = std::stoi(fields[4]); }
        catch (...) {}

        Book* book = new Book(isbn, title, author, category, copies);
        engine->addBook(book);
    }
}

void loadUsersFromCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    bool header = true;

    while (std::getline(file, line)) {
        if (header) {
            header = false;
            continue;
        }
        if (line.empty()) continue;

        auto fields = parseCSVLine(line);
        if (fields.size() < 4) continue;

        std::string uid = fields[0];
        std::string name = fields[1];
        std::string email = fields[2];
        std::string typeStr = fields[3];

        UserType type = UserType::STUDENT;
        if (typeStr == "FACULTY") type = UserType::FACULTY;
        else if (typeStr == "FINAL_YEAR_STUDENT") type = UserType::FINAL_YEAR_STUDENT;

        if (!engine->getUser(uid)) {
            engine->addUser(new User(uid, name, email, type));
        }
    }
}

void loadTransactionsFromCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return;

    std::string line;
    bool header = true;

    while (std::getline(file, line)) {
        if (header) {
            header = false;
            continue;
        }
        if (line.empty()) continue;

        auto fields = parseCSVLine(line);
        if (fields.size() < 6) continue;

        std::string tid = fields[0];
        std::string uid = fields[1];
        std::string bid = fields[2];
        std::string cid = fields[3];
        std::string typeStr = fields[4];
        long long ts = 0;
        try { ts = std::stoll(fields[5]); } catch (...) {}

        TransactionType type = (typeStr == "RETURN") ? TransactionType::RETURN : TransactionType::ISSUE;
        Transaction* t = new Transaction(tid, uid, bid, cid, type);
        t->timestamp = ts;
        engine->addTransaction(t);
    }
}

void saveUserToCSV(const std::string& path, const std::string& uid, const std::string& name, const std::string& typeStr) {
    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) return;
    
    // Simple CSV escaping if needed, for now assuming no commas in names
    file << uid << "," << name << "," << uid + "@library.edu" << "," << typeStr << "\n";
}

/* ---------------- HANDLERS ---------------- */

json handleSearch(const json& req) {
    json res;
    std::string query = req.value("query", "");
    std::string type = req.value("type", "title");

    std::vector<SearchResult> results =
        (type == "author")
        ? engine->searchByAuthor(query)
        : engine->searchByTitle(query);

    res["success"] = true;
    res["count"] = results.size();
    res["results"] = json::array();

    for (const auto& r : results) {
        res["results"].push_back({
            {"isbn", r.isbn},
            {"title", r.title},
            {"author", r.author},
            {"category", r.category},
            {"availableCopies", r.availableCopies},
            {"totalCopies", r.totalCopies},
            {"relevanceScore", r.relevanceScore}
        });
    }
    return res;
}

json handleIssue(const json& req) {
    return engine->issueBook(
        req.value("userID", ""),
        req.value("isbn", "")
    );
}

json handleReturn(const json& req) {
    return engine->returnBook(
        req.value("userID", ""),
        req.value("isbn", "")
    );
}

json handleReserve(const json& req) {
    return engine->reserveBook(
        req.value("userID", ""),
        req.value("isbn", "")
    );
}

json handleRecommend(const json& req) {
    json res;
    auto results = engine->getRecommendations(
        req.value("isbn", ""),
        req.value("limit", 5)
    );

    res["success"] = true;
    res["count"] = results.size();
    res["results"] = json::array();

    for (const auto& r : results) {
        res["results"].push_back({
            {"isbn", r.isbn},
            {"title", r.title},
            {"author", r.author},
            {"category", r.category},
            {"availableCopies", r.availableCopies},
            {"totalCopies", r.totalCopies}
        });
    }
    return res;
}

json handlePersonalizedRecommend(const json& req) {
    json res;
    std::string userID = req.value("userID", "");
    int limit = req.value("limit", 6);

    std::vector<std::string> recentISBNs;
    if (req.find("recentISBNs") != req.end() && req["recentISBNs"].is_array()) {
        for (const auto& item : req["recentISBNs"])
            recentISBNs.push_back(item.get<std::string>());
    }

    auto results = engine->getPersonalizedRecommendations(userID, recentISBNs, limit);

    res["success"] = true;
    res["count"] = results.size();
    res["results"] = json::array();

    for (const auto& r : results) {
        res["results"].push_back({
            {"isbn", r.isbn},
            {"title", r.title},
            {"author", r.author},
            {"category", r.category},
            {"availableCopies", r.availableCopies},
            {"totalCopies", r.totalCopies},
            {"relevanceScore", r.relevanceScore}
        });
    }
    return res;
}

/* ---------------- MAIN ---------------- */

int main(int argc, char* argv[]) {
    engine = new LibraryEngine();

    std::string booksPath = "data/books.csv";
    std::string usersPath = "data/users.csv";
    std::string transPath = "data/transactions.csv";

    if (argc >= 4) {
        booksPath = argv[1];
        usersPath = argv[2];
        transPath = argv[3];
    }

    loadBooksFromCSV(booksPath);
    loadUsersFromCSV(usersPath);
    loadTransactionsFromCSV(transPath);
    engine->buildSearchIndices();
    engine->buildRecommendationGraph();

    std::cout << "Library System Ready" << std::endl;
    std::cout.flush();

    std::string input;
    while (std::getline(std::cin, input)) {
        if (input.empty()) continue;
        
        try {
            json request = json::parse(input);
            json response;

            std::string action = request.value("action", "");

            if (action == "search") response = handleSearch(request);
            else if (action == "issue") response = handleIssue(request);
            else if (action == "return") response = handleReturn(request);
            else if (action == "reserve") response = handleReserve(request);
            else if (action == "recommendations") response = handleRecommend(request);
            else if (action == "personalized_recommendations") response = handlePersonalizedRecommend(request);
            else if (action == "undo") response = engine->undoLastAction();
            else if (action == "profile") response = engine->getUserProfile(request.value("userID", ""));
            else if (action == "add_user") {
                std::string uid = request.value("userID", "");
                std::string fname = request.value("name", "");
                std::string utypeStr = request.value("type", "student");
                UserType utype = UserType::STUDENT;
                if (utypeStr == "faculty") utype = UserType::FACULTY;
                else if (utypeStr == "final_year") utype = UserType::FINAL_YEAR_STUDENT;
                
                // Check if user exists first to update or add
                if (!engine->getUser(uid)) {
                    engine->addUser(new User(uid, fname, uid + "@library.edu", utype));
                    
                    // Persist to CSV (if path is provided/known)
                    std::string saveType = "STUDENT";
                    if (utype == UserType::FACULTY) saveType = "FACULTY";
                    else if (utype == UserType::FINAL_YEAR_STUDENT) saveType = "FINAL_YEAR_STUDENT";
                    saveUserToCSV(usersPath, uid, fname, saveType);
                }
                response = { {"success", true}, {"message", "User added/verified"} };
            }
            else response = { {"success", false}, {"message", "Invalid action"} };

            std::cout << response.dump() << std::endl;
            std::cout.flush();
        } catch (const std::exception& e) {
            json error = { {"success", false}, {"message", std::string("Error: ") + e.what()} };
            std::cout << error.dump() << std::endl;
            std::cout.flush();
        }
    }

    delete engine;
    return 0;
}
