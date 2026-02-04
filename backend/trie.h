#ifndef TRIE_H
#define TRIE_H

#include "models.h"
#include <unordered_map>
#include <vector>
#include <set>

struct TrieNode {
    TrieNode* children[26];
    bool isEnd;
    long long frequency;
    long long borrowImpact;
    std::set<std::string> bookIDs;

    TrieNode() : isEnd(false), frequency(0), borrowImpact(0) {
        for (int i = 0; i < 26; i++)
            children[i] = nullptr;
    }
};

class AdaptiveTrie {
private:
    TrieNode* root;

    void collect(TrieNode* node, std::set<std::string>& result);
    void deleteNode(TrieNode* node);
    int idx(char c);

public:
    AdaptiveTrie();
    ~AdaptiveTrie();

    void insert(const std::string& word, const std::string& bookID);

    std::vector<SearchResult> searchPrefix(
        const std::string& prefix,
        std::unordered_map<std::string, Book*>& bookMap
    );

    void updateBorrowImpact(const std::string& word, double value);
};

#endif
