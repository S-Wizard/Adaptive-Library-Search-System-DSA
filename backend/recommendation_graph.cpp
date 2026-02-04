#include "recommendation_graph.h"
#include <algorithm>

RecommendationGraph::RecommendationGraph() {}
RecommendationGraph::~RecommendationGraph() {}

void RecommendationGraph::addEdge(const std::string& a, const std::string& b) {
    adj[a].push_back(b);
    adj[b].push_back(a);
}

void RecommendationGraph::buildFromBooks(
    std::unordered_map<std::string, Book*>& books
) {
    std::unordered_map<std::string, std::vector<std::string>> byCategory;

    for (auto& p : books)
        byCategory[p.second->category].push_back(p.first);

    for (auto& cat : byCategory) {
        auto& ids = cat.second;
        for (size_t i = 0; i < ids.size(); i++)
            for (size_t j = i + 1; j < ids.size(); j++)
                addEdge(ids[i], ids[j]);
    }
}

void RecommendationGraph::bfs(
    const std::string& start,
    int maxDepth,
    std::vector<SearchResult>& out,
    std::unordered_map<std::string, Book*>& bookMap
) {
    std::unordered_set<std::string> visited;
    std::queue<std::pair<std::string, int>> q;

    q.push(std::make_pair(start, 0));
    visited.insert(start);

    while (!q.empty()) {
        std::pair<std::string, int> p = q.front();
        q.pop();

        std::string curr = p.first;
        int depth = p.second;

        if (depth >= maxDepth) continue;

        for (const auto& next : adj[curr]) {
            if (visited.count(next)) continue;
            visited.insert(next);

            auto it = bookMap.find(next);
            if (it != bookMap.end()) {
                Book* b = it->second;
                SearchResult r;
                r.bookID = next;
                r.isbn = b->isbn;
                r.title = b->title;
                r.author = b->author;
                r.category = b->category;
                r.availableCopies = b->availableCopies;
                r.totalCopies = b->totalCopies;
                r.relevanceScore = b->borrowImpact;
                out.push_back(r);
            }

            q.push(std::make_pair(next, depth + 1));
        }
    }
}

std::vector<SearchResult> RecommendationGraph::getRecommendations(
    const std::string& isbn,
    int limit,
    std::unordered_map<std::string, Book*>& bookMap
) {
    std::vector<SearchResult> results;
    bfs(isbn, 2, results, bookMap);

    std::sort(results.begin(), results.end(),
        [](const SearchResult& a, const SearchResult& b) {
            return a.relevanceScore > b.relevanceScore;
        });

    if ((int)results.size() > limit)
        results.resize(limit);

    return results;
}
