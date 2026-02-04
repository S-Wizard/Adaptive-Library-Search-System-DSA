#ifndef RECOMMENDATION_GRAPH_H
#define RECOMMENDATION_GRAPH_H

#include "models.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>

class RecommendationGraph {
private:
    std::unordered_map<std::string, std::vector<std::string>> adj;

    void bfs(
        const std::string& start,
        int maxDepth,
        std::vector<SearchResult>& out,
        std::unordered_map<std::string, Book*>& bookMap
    );

public:
    RecommendationGraph();
    ~RecommendationGraph();

    void addEdge(const std::string& a, const std::string& b);

    // NEW – used by LibraryEngine
    void buildFromBooks(std::unordered_map<std::string, Book*>& books);

    std::vector<SearchResult> getRecommendations(
        const std::string& isbn,
        int limit,
        std::unordered_map<std::string, Book*>& bookMap
    );
};

#endif
