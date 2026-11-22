#pragma once
#include "BinaryTree.h"

struct BSTNode : TreeNode {
    int key;
    int payload;

    BSTNode(int id, int key, int payload)
        : TreeNode(id, key), key(key), payload(payload) {
    }
};

class BST : public BinaryTree
{
protected:
    int nextID;
    BSTNode* bstRoot() const;
private:

    BSTNode* searchHelper(BSTNode* node, int key);
    BSTNode* findMinHelper(BSTNode* node);
    BSTNode* findMaxHelper(BSTNode* node);
    BSTNode* kthSmallestHelper(BSTNode* node, int k, int& count);

    bool removeHelper(TreeNode*& node, int key, EventLog* log, int parentId, int side);

    void rangeQueryHelper(BSTNode* node, int lo, int hi, List*& tail, EventLog* log);

public:
    long long compareCount;

    BST();

    void resetStats() { compareCount = 0; }

    void reset();   // delete the tree and restart node ids at 1 

    void adopt(TreeNode* r, int nextId);

    virtual bool insert(int key, EventLog* log = nullptr);

    BSTNode* search(int key, EventLog* log = nullptr);

    BSTNode* findMin();
    BSTNode* findMax();

    BSTNode* successor(int key);
    BSTNode* predecessor(int key);

    virtual bool remove(int key, EventLog* log = nullptr);

    List* rangeQuery(int lo, int hi, EventLog* log = nullptr);

    BSTNode* kthSmallest(int k);

    BSTNode* lcaBST(int a, int b);
};