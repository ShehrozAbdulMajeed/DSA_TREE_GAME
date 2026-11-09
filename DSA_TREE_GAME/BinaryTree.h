#pragma once
#include"Events.h"

struct TreeNode {
	int id;
	int val;
	TreeNode* left;
	TreeNode* right;
	TreeNode();
	TreeNode(int d, int v);
};

struct List {
	int val;
	int id;
	List* next;
	List();
	List(int d, int v);
};

struct stack {
	TreeNode* node;
	stack* next;
	stack(TreeNode* n);
};

struct treeStack {
	stack* top;
	treeStack();
	treeStack(TreeNode* n);
	void push(TreeNode* n);
	void pop();
	bool isEmpty();
	TreeNode* peek();
	~treeStack();
};

struct queue {
	TreeNode* node;
	queue* next;
	queue(TreeNode* n);
};

struct treeQueue {
	queue* top;
	queue* rear;
	treeQueue();
	treeQueue(TreeNode* n);
	void enqueue(TreeNode* n);
	void dequeue();
	TreeNode* peek();
	bool isEmpty();
	~treeQueue();
};

class BinaryTree {
protected:
	TreeNode* root;
private:
	int max(int a, int b);

	TreeNode* postRev(List*& postOrder, List* inOrder);

	int diamCalc(TreeNode* node);

	TreeNode* lca_help(TreeNode* node, int a, int b);

	bool pathto_help(TreeNode* node, int id, List*& path);

	void mirror_help(TreeNode* node);

	bool isSymmetric_help(TreeNode* left, TreeNode* right);

	bool isSameTree_help(TreeNode* a, TreeNode* b);

	void preOrder_R_H(TreeNode* node, List*& link, EventLog* log);

	void postOrder_R_H(TreeNode* node, List*& link, EventLog* log);

	void inOrder_R_H(TreeNode* node, List*& link, EventLog* log);

	void levelOrder_R_H(TreeNode* node, List*& link, int level, EventLog* log);

	bool checkID(TreeNode* node, int id);

public:
	BinaryTree();

	TreeNode* preBuild(List*& preOrder, List* inOrder);

	TreeNode* postBuild(List*& postOrder, List* inOrder);

	void traverse(TreeNode* node);

	int height(TreeNode* node);

	int diameter();

	TreeNode* lca(int a, int b);

	List* pathto(TreeNode* node);

	void mirror();

	bool isSymmetric();

	bool isSameTree(TreeNode* a);

	List* preOrder_R(EventLog* log = nullptr);

	List* postOrder_R(EventLog* log = nullptr);

	List* inOrder_R(EventLog* log = nullptr);

	void setTree(TreeNode* node);

	TreeNode* getTree() const;

	List* levelOrder_R(EventLog* log = nullptr);

	List* preOrder_I(EventLog* log = nullptr);

	List* inOrder_I(EventLog* log = nullptr);

	List* postOrder_I(EventLog* log = nullptr);

	List* levelOrder_I(EventLog* log = nullptr);
};