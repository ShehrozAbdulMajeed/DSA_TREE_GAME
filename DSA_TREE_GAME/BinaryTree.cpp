#include<iostream>
using namespace std;

struct TreeNode {
	int id;
	int val;
	TreeNode* left;
	TreeNode* right;
	TreeNode(int d, int v) {
		val = v;
		left = nullptr;
		right = nullptr;
		id = d;
	}
};

struct List {
	int val;
	int id;
	List* next;
	List(){
	val = 0;
	id = 0;
	next = nullptr;
	}
	List(int d, int v) {
		val = v;
		id = d;
		next = nullptr;
	}
};

TreeNode* preBuild(List*& preOrder, List* inOrder) {
	if(!preOrder || !inOrder) return nullptr;
	TreeNode* root = new TreeNode(preOrder->id, preOrder->val);
	preOrder = preOrder->next;
	List* prev = nullptr;
	List* curr = inOrder;
	while (curr && curr->id != root->id) {
		prev = curr;
		curr = curr->next;
	}
	List* right = nullptr;
	if(curr != nullptr) right = curr->next;
	if(prev != nullptr) prev->next = nullptr;
	else inOrder = nullptr;
	root->left = preBuild(preOrder,inOrder);
	root->right = preBuild(preOrder,right);
	if(prev != nullptr) prev->next = curr;
	return root;
}

TreeNode* postRev(List*& postOrder, List* inOrder) {
	if (!postOrder || !inOrder) return nullptr;
	TreeNode* root = new TreeNode(postOrder->id, postOrder->val);
	postOrder = postOrder->next;
	List* prev = nullptr;
	List* curr = inOrder;
	while (curr && curr->id != root->id) {
		prev = curr;
		curr = curr->next;
	}
	List* right = nullptr;
	if (curr != nullptr) right = curr->next;
	if (prev != nullptr) prev->next = nullptr;
	else inOrder = nullptr;
	root->right = postRev(postOrder, right);
	root->left = postRev(postOrder, inOrder);
	if (prev != nullptr) prev->next = curr;
	return root;
	
}

TreeNode* postBuild(List*& postOrder, List* inOrder) {
  List* temp = new List(postOrder->id, postOrder->val);
  List* curr_1 = postOrder;
  List* curr_t = temp;
  while (curr_1->next != nullptr) {
	  curr_1 = curr_1->next;
	  curr_t->next = new List(curr_1->id, curr_1->val);
	  curr_t = curr_t->next;
  }
  List* prev = nullptr;
  List* curr = temp;
  List* next = nullptr;
  while (curr != nullptr) {
	  next = curr->next;
	  curr->next = prev;
	  prev = curr;
	  curr = next;
  }
  temp = prev;
  return postRev(temp,inOrder);
}

void traverse(TreeNode* root) {
	if(root == nullptr) return;
	traverse(root->left);
	cout << root->id << " ";
	traverse(root->right);
}

int main() {
	List* pre = new List(5,30);
	pre->next = new List(2,35); 
	pre->next->next = new List(1,40);
	pre->next->next->next = new List(8,90);
	pre->next->next->next->next = new List(11,60);

	List* in = new List(11,40); 
	in->next = new List(8,35);
	in->next->next = new List(1,90);
	in->next->next->next = new List(2,30);
	in->next->next->next->next = new List(5,60);

	List* po = new List(11,90);
	po->next = new List(8, 30);
	po->next->next = new List(1, 35);
	po->next->next->next = new List(2,60);
	po->next->next->next->next = new List(5, 40);
	List* temp = pre;
	//TreeNode* root = preBuild(pre,in);
	TreeNode* root = postBuild(po,in);
	traverse(root);
	pre = temp;
}