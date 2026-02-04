#ifndef AVL_TREE_H
#define AVL_TREE_H

#include "models.h"
#include <string>
#include <vector>   // 🔥 REQUIRED
#include <algorithm>

struct AVLNode {
    std::string key;
    Book* book;
    int height;
    AVLNode* left;
    AVLNode* right;

    AVLNode(const std::string& k, Book* b)
        : key(k), book(b), height(1), left(nullptr), right(nullptr) {}
};

class AVLTree {
private:
    AVLNode* root;

    int height(AVLNode* node);
    int getBalance(AVLNode* node);
    AVLNode* rightRotate(AVLNode* y);
    AVLNode* leftRotate(AVLNode* x);

    AVLNode* insertNode(AVLNode* node, const std::string& key, Book* book);
    AVLNode* searchNode(AVLNode* node, const std::string& key);
    void inorderTraversal(AVLNode* node, std::vector<Book*>& books);

public:
    AVLTree();
    ~AVLTree();

    void insert(const std::string& key, Book* book);
    Book* search(const std::string& key);
    std::vector<Book*> getAllBooks();
};

#endif
