#include "avl_tree.h"

AVLTree::AVLTree() : root(nullptr) {}
AVLTree::~AVLTree() {}

int AVLTree::height(AVLNode* n) {
    return n ? n->height : 0;
}

int AVLTree::getBalance(AVLNode* n) {
    return n ? height(n->left) - height(n->right) : 0;
}

AVLNode* AVLTree::rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = std::max(height(y->left), height(y->right)) + 1;
    x->height = std::max(height(x->left), height(x->right)) + 1;

    return x;
}

AVLNode* AVLTree::leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = std::max(height(x->left), height(x->right)) + 1;
    y->height = std::max(height(y->left), height(y->right)) + 1;

    return y;
}

AVLNode* AVLTree::insertNode(AVLNode* node, const std::string& key, Book* book) {
    if (!node) return new AVLNode(key, book);

    if (key < node->key)
        node->left = insertNode(node->left, key, book);
    else if (key > node->key)
        node->right = insertNode(node->right, key, book);
    else
        return node;

    node->height = 1 + std::max(height(node->left), height(node->right));

    int balance = getBalance(node);

    if (balance > 1 && key < node->left->key)
        return rightRotate(node);
    if (balance < -1 && key > node->right->key)
        return leftRotate(node);
    if (balance > 1 && key > node->left->key) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && key < node->right->key) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

void AVLTree::insert(const std::string& key, Book* book) {
    root = insertNode(root, key, book);
}

AVLNode* AVLTree::searchNode(AVLNode* node, const std::string& key) {
    if (!node || node->key == key) return node;
    if (key < node->key)
        return searchNode(node->left, key);
    return searchNode(node->right, key);
}

Book* AVLTree::search(const std::string& key) {
    AVLNode* n = searchNode(root, key);
    return n ? n->book : nullptr;
}

void AVLTree::inorderTraversal(AVLNode* node, std::vector<Book*>& books) {
    if (!node) return;
    inorderTraversal(node->left, books);
    books.push_back(node->book);
    inorderTraversal(node->right, books);
}

std::vector<Book*> AVLTree::getAllBooks() {
    std::vector<Book*> books;
    inorderTraversal(root, books);
    return books;
}
