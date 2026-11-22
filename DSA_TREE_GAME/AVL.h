#pragma once
#include"BST.h"

struct AVLNode : BSTNode {
    int height = 1;   // leaf = 1
    int size = 1;     // leaf = 1

    AVLNode(int id, int key, int payload) : BSTNode(id, key, payload) {}
};

class AVL : public BST {
private:
    AVLNode* insertHelper(AVLNode* node, int key, int payload, EventLog* log,
        bool& inserted, int parentId, int side);
    bool isBalancedHelper(AVLNode* node);
    AVLNode* findMinHelper(AVLNode* node);
    bool removeHelper(TreeNode*& node, int key, EventLog* log, int parentId, int side);
    AVLNode* kthHelper(AVLNode* node, int k, EventLog* log);
    int rankHelper(AVLNode* node, int key, EventLog* log);
    void updateHeight(AVLNode* node, EventLog* log);
    void updateSize(AVLNode* node, EventLog* log);
    void recomputeHelper(AVLNode* node);
    AVLNode* findWithParent(AVLNode* cur, int id, AVLNode* par, AVLNode*& outParent);
    bool refreshAbove(AVLNode* cur, int id, EventLog* log);
public:
    long long rotationCount;   // benchmark hook: single rotations performed (a double = 2)
    bool balancing;            // false = inserts/deletes update heights/sizes 

    AVL() : BST(), rotationCount(0), balancing(true) {}

    void resetStats() { compareCount = 0; rotationCount = 0; }

    int getHeight(AVLNode* node);
    int getSize(AVLNode* node);
    int balanceFactor(AVLNode* node);
    AVLNode* rotateLeft(AVLNode* node, EventLog* log = nullptr);
    AVLNode* rotateRight(AVLNode* node, EventLog* log = nullptr);

    bool insert(int key, int payload, EventLog* log = nullptr);
    // Overrides BST::insert / BST::remove 
    bool insert(int key, EventLog* log = nullptr) override { return insert(key, 0, log); }
    bool remove(int key, EventLog* log = nullptr) override;

    AVLNode* kth(int k, EventLog* log = nullptr);
    int rank(int key, EventLog* log = nullptr);
    int countInRange(int lo, int hi, EventLog* log = nullptr);
    bool isBalanced();
    void recompute();          // recalculates every height and size 
    AVLNode* rebalance(AVLNode* node, EventLog* log = nullptr);
    bool rotateAt(int nodeId, bool left, EventLog* log = nullptr);
};