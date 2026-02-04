#include "trie.h"
#include <algorithm>
#include <cctype>

AdaptiveTrie::AdaptiveTrie() {
    root = new TrieNode();
}

AdaptiveTrie::~AdaptiveTrie() {
    deleteNode(root);
}

int AdaptiveTrie::idx(char c) {
    c = std::tolower(static_cast<unsigned char>(c));
    if (c < 'a' || c > 'z') return -1;
    return c - 'a';
}

void AdaptiveTrie::deleteNode(TrieNode* node) {
    if (!node) return;
    for (int i = 0; i < 26; i++)
        deleteNode(node->children[i]);
    delete node;
}

void AdaptiveTrie::insert(const std::string& word, const std::string& bookID) {
    TrieNode* curr = root;
    for (char c : word) {
        int i = idx(c);
        if (i < 0) continue;
        if (!curr->children[i])
            curr->children[i] = new TrieNode();
        curr = curr->children[i];
    }
    curr->isEnd = true;
    curr->bookIDs.insert(bookID);
}

void AdaptiveTrie::collect(TrieNode* node, std::set<std::string>& result) {
    if (!node) return;
    for (auto& id : node->bookIDs)
        result.insert(id);
    for (int i = 0; i < 26; i++)
        collect(node->children[i], result);
}

std::vector<SearchResult> AdaptiveTrie::searchPrefix(
    const std::string& prefix,
    std::unordered_map<std::string, Book*>& bookMap
) {
    TrieNode* curr = root;

    for (char c : prefix) {
        int i = idx(c);
        if (i < 0 || !curr->children[i])
            return {};
        curr = curr->children[i];
    }

    std::set<std::string> ids;
    collect(curr, ids);

    std::vector<SearchResult> results;
    for (auto& id : ids) {
        auto it = bookMap.find(id);
        if (it == bookMap.end()) continue;

        Book* b = it->second;
        SearchResult r;
        r.bookID = id;
        r.isbn = b->isbn;
        r.title = b->title;
        r.author = b->author;
        r.category = b->category;
        r.availableCopies = b->availableCopies;
        r.totalCopies = b->totalCopies;
        r.relevanceScore = b->borrowImpact;

        results.push_back(r);
    }

    std::sort(results.begin(), results.end(),
        [](const SearchResult& a, const SearchResult& b) {
            return a.relevanceScore > b.relevanceScore;
        });

    return results;
}

void AdaptiveTrie::updateBorrowImpact(const std::string& word, double value) {
    TrieNode* curr = root;
    for (char c : word) {
        int i = idx(c);
        if (i < 0 || !curr->children[i])
            return;
        curr = curr->children[i];
    }
    curr->borrowImpact += value;
}
