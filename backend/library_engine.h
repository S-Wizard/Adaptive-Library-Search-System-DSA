#ifndef LIBRARY_ENGINE_H
#define LIBRARY_ENGINE_H

#include "avl_tree.h"
#include "trie.h"
#include "recommendation_graph.h"
#include "models.h"

#include <unordered_map>
#include <queue>
#include <stack>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ReservationCompare {
    bool operator()(const Reservation* a, const Reservation* b) const {
        if (a->priorityScore != b->priorityScore)
            return a->priorityScore < b->priorityScore;
        return a->timestamp > b->timestamp;
    }
};

class LibraryEngine {
private:
    AVLTree bookISBNIndex;
    AdaptiveTrie titleTrie;
    AdaptiveTrie authorTrie;
    RecommendationGraph recommendations;

    std::unordered_map<std::string, Book*> books;
    std::unordered_map<std::string, User*> users;

    std::unordered_map<
        std::string,
        std::priority_queue<Reservation*, std::vector<Reservation*>, ReservationCompare>
    > reservationQueues;

    std::stack<Transaction*> transactionHistory;

    int transactionCounter;
    int reservationCounter;

public:
    LibraryEngine();
    ~LibraryEngine();

    // Books
    void addBook(Book* book);
    Book* getBook(const std::string& isbn);

    // Users
    void addUser(User* user);
    User* getUser(const std::string& userID);

    // Search
    std::vector<SearchResult> searchByTitle(const std::string& query);
    std::vector<SearchResult> searchByAuthor(const std::string& query);

    // Circulation
    json issueBook(const std::string& userID, const std::string& isbn);
    json returnBook(const std::string& userID, const std::string& isbn);
    json reserveBook(const std::string& userID, const std::string& isbn);

    // Recommendations
    std::vector<SearchResult> getRecommendations(const std::string& isbn, int limit);
    std::vector<SearchResult> getPersonalizedRecommendations(
        const std::string& userID,
        const std::vector<std::string>& recentISBNs,
        int limit
    );

    // Undo
    json undoLastAction();

    // Profile
    json getUserProfile(const std::string& userID);

    // Setup
    void buildSearchIndices();
    void buildRecommendationGraph();
};

#endif
