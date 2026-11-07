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

struct BinaryTree {
	TreeNode* root;

	TreeNode* preBuild(List*& preOrder, List* inOrder) {
		if (!preOrder || !inOrder) return nullptr;
		TreeNode* root = new TreeNode(preOrder->id, preOrder->val);
		preOrder = preOrder->next;
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
		root->left = preBuild(preOrder, inOrder);
		root->right = preBuild(preOrder, right);
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
		return postRev(temp, inOrder);
	}

	void traverse(TreeNode* root) {
		if (root == nullptr) return;
		traverse(root->left);
		cout << root->id << " ";
		traverse(root->right);
	}

	int max(int a, int b) {
		if (a > b) return a;
		return b;
	}

	int height(TreeNode* node) {
		if (node == nullptr) return 0;
		int h_left = 1 + height(node->left);
		int h_right = 1 + height(node->right);
		return max(h_left, h_right);
	}

	int diamCalc(TreeNode* node) {
	if(node == nullptr) return 0;
	int left = diamCalc(node->left);
	int right = diamCalc(node->right);
	int res = height(node->left)+height(node->right)+1;
	return max(res,max(left,right));
	}

	int diameter() {
	return diamCalc(root);
	}

	TreeNode* lca_help(TreeNode* node, int a, int b) {
	if(node == nullptr) return nullptr;
	if(node->id == a || node->id == b) return node;
	TreeNode* left = lca_help(node->left,a,b);
	TreeNode* right = lca_help(node->right,a,b);
	if(left != nullptr && right != nullptr) return node;
	if(left != nullptr) return left;
	if(right != nullptr) return right;
	else return nullptr;
	}

	TreeNode* lca(int a, int b) {
	return lca_help(root,a,b);
	}

	bool pathto_help(TreeNode* node, int id, List*& path) {
	if(node == nullptr) return false;
	if (node->id == id) {
		List* temp = new List(node->id, node->val);
		temp->next = path;
		path = temp;
		return true;
	}
	bool left = pathto_help(node->left,id,path);
	bool right = pathto_help(node->right,id,path);
	if(!left && !right) return false;
	List* temp = new List(node->id,node->val);
	temp->next = path;
	path = temp;
	return true;
	}

	List* pathto(TreeNode* node) {
	if(root == nullptr || node == nullptr) return nullptr;
	List* temp = nullptr;
	pathto_help(root, node->id, temp);
	return temp;
	}

	void mirror_help(TreeNode* node) {
	if(node == nullptr) return;
	TreeNode* temp = node->right;
	node->right = node->left;
	node->left = temp;
	mirror_help(node->left);
	mirror_help(node->right);
	}

	void mirror() {
	mirror_help(root);
	}

	bool isSymmetric_help(TreeNode* left, TreeNode* right) {
	bool l,r;
	if(left == nullptr && right == nullptr) return true;
	if(right == nullptr) return false;
	if(left == nullptr) return false;
	if (left->val == right->val) {
		 l = isSymmetric_help(left->left, right->right);
		 r = isSymmetric_help(left->right, right->left);
	}
	else return false;
		if(l && r) return true;
		else return false;
	}

	bool isSymmetric() {
	if(root == nullptr) return false;
	return isSymmetric_help(root->left,root->right);
	}

	bool isSameTree_help(TreeNode* a, TreeNode* b) {
		bool l, r;
		if (a == nullptr && b == nullptr) return true;
		if (a == nullptr) return false;
		if (b == nullptr) return false;
		if (a->val == b->val) {
			l = isSameTree_help(a->left, b->left);
			r = isSameTree_help(a->right, b->right);
		}
		else return false;
		if (l && r) return true;
		else return false;
	}

	bool isSameTree(TreeNode* a) {
	return isSameTree_help(a,root);
	}

	void preOrder_R_H(TreeNode* node,List*& link) {
	if(node == nullptr) return;
	List* temp = new List(node->id,node->val);
	temp->next = link;
	link = temp;
	preOrder_R_H(node->left,link);
	preOrder_R_H(node->right,link);
	}

	List* preOrder_R() {
		if(root == nullptr) return nullptr;
		List* temp = nullptr;
		preOrder_R_H(root,temp);
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
		return temp;
	}

	void postOrder_R_H(TreeNode* node, List*& link) {
		if (node == nullptr) return;
		postOrder_R_H(node->left, link);
		postOrder_R_H(node->right, link);
		List* temp = new List(node->id, node->val);
		temp->next = link;
		link = temp;
	}

	List* postOrder_R() {
		if (root == nullptr) return nullptr;
		List* temp = nullptr;
		postOrder_R_H(root, temp);
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
		return temp;
	}

	void inOrder_R_H(TreeNode* node, List*& link) {
		if (node == nullptr) return;
		inOrder_R_H(node->left, link);
		List* temp = new List(node->id, node->val);
		temp->next = link;
		link = temp;
		inOrder_R_H(node->right, link);
	}

	List* inOrder_R() {
		if (root == nullptr) return nullptr;
		List* temp = nullptr;
		inOrder_R_H(root, temp);
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
		return temp;
	}
};

int main() {
	List* pre = new List(5,30);
	pre->next = new List(2,35); 
	pre->next->next = new List(1,40);
	pre->next->next->next = new List(8,90);
	pre->next->next->next->next = new List(11,60);

	List* in = new List(2,2); 
	in->next = new List(1,1);
	in->next->next = new List(5,5);
	in->next->next->next = new List(4,1);
	in->next->next->next->next = new List(3,2);

	List* po = new List(2,2);
	po->next = new List(1, 1);
	po->next->next = new List(3,2);
	po->next->next->next = new List(4,1);
	po->next->next->next->next = new List(5, 5);
	List* temp = pre;
	BinaryTree* tree = new BinaryTree;
	tree->root = tree->postBuild(po,in);
	tree->traverse(tree->root);
	pre = temp;
	cout << '\n';
	cout << tree->height(tree->root);
	cout << '\n' << tree->diameter() << '\n';
	TreeNode* t1 = tree->lca(8,11);
	cout << tree->isSymmetric();
}