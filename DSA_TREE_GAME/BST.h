#pragma once
#include "BinaryTree.h"

class BST : public BinaryTree
{
private:
    int nextID;

    TreeNode* searchHelper(TreeNode* node, int key);
    TreeNode* findMinHelper(TreeNode* node);
    TreeNode* findMaxHelper(TreeNode* node);
    TreeNode* kthSmallestHelper(TreeNode* node, int k, int& count);

    bool removeHelper(TreeNode*& node, int key, EventLog* log);

    void rangeQueryHelper(TreeNode* node, int lo, int hi, List*& tail, EventLog* log);

public:
    BST();

    bool insert(int key, EventLog* log = nullptr);

    TreeNode* search(int key, EventLog* log = nullptr);

    TreeNode* findMin();
    TreeNode* findMax();

    TreeNode* successor(int key);
    TreeNode* predecessor(int key);

    bool remove(int key, EventLog* log = nullptr);

    List* rangeQuery(int lo, int hi, EventLog* log = nullptr);

    TreeNode* kthSmallest(int k);

    TreeNode* lcaBST(int a, int b);
};
