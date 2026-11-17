#pragma once
#include"BST.h"

struct AVLNode : BSTNode {
    int height = 1;
    int size = 1;

    AVLNode(int id, int key, int payload) : BSTNode(id, key, payload) {}
};

class AVL : public BST {
private:
    AVLNode* insertHelper(AVLNode* node, int key, int payload, EventLog* log, bool& check);
    bool isBalancedHelper(AVLNode* node);
    AVLNode* findMinHelper(AVLNode* node);
    bool removeHelper(TreeNode*& node, int key, EventLog* log = nullptr);
    AVLNode* kthHelper(AVLNode* node, int k);
    void updateHeight(AVLNode* node);
    void updateSize(AVLNode* node);
    int rankHelper(AVLNode* node, int key);
    public:
    int getHeight(AVLNode* node);
    int getSize(AVLNode* node);
    int balanceFactor(AVLNode* node);
    AVLNode* rotateLeft(AVLNode* node);
    AVLNode* rotateRight(AVLNode* node);
    bool insert(int key, int payload, EventLog* log = nullptr);
    bool remove(int key, EventLog* log = nullptr);
    AVLNode* kth(int k);
    int rank(int key);
    int countInRange(int lo, int hi);
    bool isBalanced();
    AVLNode* rebalance(AVLNode* node);
};