#include"BinaryTree.h"

TreeNode::TreeNode() {
	val = 0;
	left = nullptr;
	right = nullptr;
	id = 0;
}

TreeNode::TreeNode(int d, int v) {
	val = v;
	left = nullptr;
	right = nullptr;
	id = d;
}

List::List() {
	val = 0;
	id = 0;
	next = nullptr;
}

List::List(int d, int v) {
	val = v;
	id = d;
	next = nullptr;
}

void freeList(List* head) {
	while (head != nullptr) {
		List* n = head->next;
		delete head;
		head = n;
	}
}

StackNode::StackNode(TreeNode* n) {
	node = n;
	next = nullptr;
}

treeStack::treeStack() {
	top = nullptr;
}

treeStack::treeStack(TreeNode* n) {
	top = new StackNode(n);
}

void treeStack::push(TreeNode* n) {
	StackNode* temp = new StackNode(n);
	temp->next = top;
	top = temp;
}

void treeStack::pop() {
	if (top == nullptr) return;
	StackNode* temp = top;
	top = top->next;
	delete temp;
	temp = nullptr;
}

bool treeStack::isEmpty() {
	if (top == nullptr) return true;
	return false;
}

TreeNode* treeStack::peek() {
	if (top == nullptr) return nullptr;
	return top->node;
}

treeStack::~treeStack() {
	while (!isEmpty())
		pop();
}

QueueNode::QueueNode(TreeNode* n) {
	node = n;
	next = nullptr;
}

treeQueue::treeQueue() {
	top = nullptr;
	rear = nullptr;
}

treeQueue::treeQueue(TreeNode* n) {
	top = new QueueNode(n);
	rear = top;
}

void treeQueue::enqueue(TreeNode* n) {
	QueueNode* temp = new QueueNode(n);
	if (rear != nullptr) {
		rear->next = temp;
		rear = rear->next;
	}
	else {
		top = temp;
		rear = top;
	}
}

void treeQueue::dequeue() {
	if (top == nullptr || rear == nullptr) return;
	QueueNode* temp = top;
	top = top->next;
	if (top == nullptr) rear = nullptr;
	delete temp;
	temp = nullptr;
}

TreeNode* treeQueue::peek() {
	if (top == nullptr || rear == nullptr) return nullptr;
	return top->node;
}

bool treeQueue::isEmpty() {
	if (top == nullptr) return true;
	return false;
}

treeQueue::~treeQueue() {
	while (!isEmpty())
		dequeue();
}

int BinaryTree::max(int a, int b) {
	if (a > b) return a;
	return b;
}

TreeNode* BinaryTree::postRev(List*& postOrder, List* inOrder) {
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

int BinaryTree::diamCalc(TreeNode* node) {
	if (node == nullptr) return 0;
	int left = diamCalc(node->left);
	int right = diamCalc(node->right);
	int res = height(node->left) + height(node->right) + 1;
	return max(res, max(left, right));
}

TreeNode* BinaryTree::lca_help(TreeNode* node, int a, int b) {
	if (node == nullptr) return nullptr;
	if (node->id == a || node->id == b) return node;
	TreeNode* left = lca_help(node->left, a, b);
	TreeNode* right = lca_help(node->right, a, b);
	if (left != nullptr && right != nullptr) return node;
	if (left != nullptr) return left;
	if (right != nullptr) return right;
	else return nullptr;
}

bool BinaryTree::pathto_help(TreeNode* node, int id, List*& path) {
	if (node == nullptr) return false;
	if (node->id == id) {
		List* temp = new List(node->id, node->val);
		temp->next = path;
		path = temp;
		return true;
	}
	bool left = pathto_help(node->left, id, path);
	bool right = pathto_help(node->right, id, path);
	if (!left && !right) return false;
	List* temp = new List(node->id, node->val);
	temp->next = path;
	path = temp;
	return true;
}

void BinaryTree::mirror_help(TreeNode* node) {
	if (node == nullptr) return;
	TreeNode* temp = node->right;
	node->right = node->left;
	node->left = temp;
	mirror_help(node->left);
	mirror_help(node->right);
}

bool BinaryTree::isSymmetric_help(TreeNode* left, TreeNode* right) {
	bool l, r;
	if (left == nullptr && right == nullptr) return true;
	if (right == nullptr) return false;
	if (left == nullptr) return false;
	if (left->val == right->val) {
		l = isSymmetric_help(left->left, right->right);
		r = isSymmetric_help(left->right, right->left);
	}
	else return false;
	if (l && r) return true;
	else return false;
}

bool BinaryTree::isSameTree_help(TreeNode* a, TreeNode* b) {
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

void BinaryTree::preOrder_R_H(TreeNode* node, List*& link, EventLog* log) {
	if (node == nullptr) return;
	List* temp = new List(node->id, node->val);
	temp->next = link;
	link = temp;
	if (log != nullptr) log->add(EventTypes::VISIT, node->id);
	preOrder_R_H(node->left, link, log);
	preOrder_R_H(node->right, link, log);
}

void BinaryTree::postOrder_R_H(TreeNode* node, List*& link, EventLog* log) {
	if (node == nullptr) return;
	postOrder_R_H(node->left, link, log);
	postOrder_R_H(node->right, link, log);
	List* temp = new List(node->id, node->val);
	temp->next = link;
	link = temp;
	if (log != nullptr) log->add(EventTypes::VISIT, node->id);
}

void BinaryTree::inOrder_R_H(TreeNode* node, List*& link, EventLog* log) {
	if (node == nullptr) return;
	inOrder_R_H(node->left, link, log);
	List* temp = new List(node->id, node->val);
	temp->next = link;
	link = temp;
	if (log != nullptr) log->add(EventTypes::VISIT, node->id);
	inOrder_R_H(node->right, link, log);
}

void BinaryTree::levelOrder_R_H(TreeNode* node, List*& link, int level, EventLog* log) {
	if (node == nullptr) return;
	if (level == 1) {
		List* temp = new List(node->id, node->val);
		temp->next = link;
		link = temp;
		if (log != nullptr) log->add(EventTypes::VISIT, node->id);
	}
	levelOrder_R_H(node->left, link, level - 1, log);
	levelOrder_R_H(node->right, link, level - 1, log);

}

bool BinaryTree::checkID(TreeNode* node, int id) {
	if (node == nullptr) return false;
	if (node->id == id) return true;
	bool l = checkID(node->left, id);
	bool r = checkID(node->right, id);
	if (r || l) return true;
	return false;
}

BinaryTree::BinaryTree() {
	root = nullptr;
}

TreeNode* BinaryTree::preBuildHelper(List*& preOrder, List* inOrder) {
	if (!preOrder || !inOrder) return nullptr;
	TreeNode* temp = new TreeNode(preOrder->id, preOrder->val);
	preOrder = preOrder->next;
	List* prev = nullptr;
	List* curr = inOrder;
	while (curr && curr->id != temp->id) {
		prev = curr;
		curr = curr->next;
	}
	List* right = nullptr;
	if (curr != nullptr) right = curr->next;
	if (prev != nullptr) prev->next = nullptr;
	else inOrder = nullptr;
	temp->left = preBuildHelper(preOrder, inOrder);
	temp->right = preBuildHelper(preOrder, right);
	if (prev != nullptr) prev->next = curr;
	return temp;
}

TreeNode* BinaryTree::preBuild(List* preOrder, List* inOrder) {
	List* cursor = preOrder;   // helper advances this copy, not the caller's list
	return preBuildHelper(cursor, inOrder);
}

TreeNode* BinaryTree::postBuild(List* postOrder, List* inOrder) {
	if (postOrder == nullptr || inOrder == nullptr) return nullptr;
	// reversed copy of postorder 
	List* rev = nullptr;
	for (List* c = postOrder; c != nullptr; c = c->next) {
		List* t = new List(c->id, c->val);
		t->next = rev;
		rev = t;
	}
	List* cursor = rev;
	TreeNode* result = postRev(cursor, inOrder);
	freeList(rev);
	return result;
}

void BinaryTree::traverse(TreeNode* node) {
	if (node == nullptr) return;
	traverse(node->left);
	std::cout << node->id << " ";
	traverse(node->right);
}

int BinaryTree::height(TreeNode* node) {
	if (node == nullptr) return 0;
	int h_left = 1 + height(node->left);
	int h_right = 1 + height(node->right);
	return max(h_left, h_right);
}

int BinaryTree::diameter() {
	return diamCalc(root);
}

TreeNode* BinaryTree::lca(int a, int b) {
	if (!checkID(root, a) || !checkID(root, b)) return nullptr;
	return lca_help(root, a, b);
}

List* BinaryTree::pathto(TreeNode* node) {
	if (root == nullptr || node == nullptr) return nullptr;
	List* temp = nullptr;
	pathto_help(root, node->id, temp);
	return temp;
}

void BinaryTree::mirror() {
	mirror_help(root);
}

bool BinaryTree::isSymmetric() {
	if (root == nullptr) return true;
	return isSymmetric_help(root->left, root->right);
}

bool BinaryTree::isSameTree(TreeNode* a) {
	return isSameTree_help(a, root);
}

List* BinaryTree::preOrder_R(EventLog* log) {
	if (root == nullptr) return nullptr;
	List* temp = nullptr;
	preOrder_R_H(root, temp, log);
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

List* BinaryTree::postOrder_R(EventLog* log) {
	if (root == nullptr) return nullptr;
	List* temp = nullptr;
	postOrder_R_H(root, temp, log);
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

List* BinaryTree::inOrder_R(EventLog* log) {
	if (root == nullptr) return nullptr;
	List* temp = nullptr;
	inOrder_R_H(root, temp, log);
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

void BinaryTree::setTree(TreeNode* node) {
	if (root != node) destroyTree(root);
	root = node;
}

TreeNode* BinaryTree::getTree() const {
	return root;
}

TreeNode* BinaryTree::releaseTree() {
	TreeNode* r = root;
	root = nullptr;
	return r;
}

List* BinaryTree::levelOrder_R(EventLog* log) {
	if (root == nullptr) return nullptr;
	List* temp = nullptr;
	int h = height(root);
	for (int level = 1; level <= h; level++)
		levelOrder_R_H(root, temp, level, log);   // prepends each visited node
	List* prev = nullptr;
	List* curr = temp;
	List* next = nullptr;
	while (curr != nullptr) {
		next = curr->next;
		curr->next = prev;
		prev = curr;
		curr = next;
	}
	return prev;
}

List* BinaryTree::preOrder_I(EventLog* log) {
	if (root == nullptr) return nullptr;
	List* temp = nullptr;
	TreeNode* curr = root;
	treeStack* st = new treeStack;
	st->push(curr);
	List* curr_t = temp;
	while (!st->isEmpty()) {
		curr = st->peek();
		if (log != nullptr) log->add(EventTypes::VISIT, curr->id);
		st->pop();
		if (temp != nullptr) {
			curr_t->next = new List(curr->id, curr->val);
			curr_t = curr_t->next;
		}
		if (temp == nullptr) {
			temp = new List(curr->id, curr->val);
			curr_t = temp;
		}
		if (curr->right != nullptr) st->push(curr->right);
		if (curr->left != nullptr) st->push(curr->left);
	}
	delete st;
	return temp;
}

List* BinaryTree::inOrder_I(EventLog* log) {
	if (root == nullptr) return nullptr;
	treeStack* st = new treeStack;
	List* temp = nullptr;
	List* curr_t = temp;
	TreeNode* curr = root;
	while (!st->isEmpty() || curr != nullptr) {
		while (curr != nullptr) {
			st->push(curr);
			curr = curr->left;
		}
		curr = st->peek();
		if (log != nullptr) log->add(EventTypes::VISIT, curr->id);
		st->pop();
		if (temp != nullptr) {
			if (curr != nullptr) {
				curr_t->next = new List(curr->id, curr->val);
				curr_t = curr_t->next;
			}
		}
		if (temp == nullptr) {
			temp = new List(curr->id, curr->val);
			curr_t = temp;
		}
		curr = curr->right;
	}
	delete st;
	return temp;
}

List* BinaryTree::postOrder_I(EventLog* log) {
	if (root == nullptr) return nullptr;
	treeStack* st = new treeStack;
	List* temp = nullptr;
	List* curr_t = temp;
	TreeNode* prev = nullptr;
	st->push(root);
	while (!st->isEmpty()) {
		TreeNode* curr = st->peek();
		if ((prev != curr->left && prev != curr->right) || prev == nullptr) {
			if (curr->left != nullptr) {
				curr = curr->left;
				st->push(curr);
			}
			else if (curr->right != nullptr) {
				curr = curr->right;
				st->push(curr);
			}
			else {
				curr = st->peek();
				prev = curr;
				if (log != nullptr) log->add(EventTypes::VISIT, curr->id);
				st->pop();
				if (temp != nullptr) {
					curr_t->next = new List(curr->id, curr->val);
					curr_t = curr_t->next;
				}
				if (temp == nullptr) {
					temp = new List(curr->id, curr->val);
					curr_t = temp;
				}
			}
		}
		else if (prev == curr->left) {
			if (curr->right == nullptr) {
				curr = st->peek();
				prev = st->peek();
				if (log != nullptr) log->add(EventTypes::VISIT, curr->id);
				st->pop();
				if (temp != nullptr) {
					curr_t->next = new List(curr->id, curr->val);
					curr_t = curr_t->next;
				}
				if (temp == nullptr) {
					temp = new List(curr->id, curr->val);
					curr_t = temp;
				}

			}
			else if (curr->right != nullptr)
				st->push(curr->right);
		}
		else if (prev == curr->right) {
			curr = st->peek();
			prev = st->peek();
			st->pop();
			if (log != nullptr) log->add(EventTypes::VISIT, curr->id);
			if (temp != nullptr) {
				curr_t->next = new List(curr->id, curr->val);
				curr_t = curr_t->next;
			}
			if (temp == nullptr) {
				temp = new List(curr->id, curr->val);
				curr_t = temp;
			}


		}

	}
	delete st;
	return temp;
}

List* BinaryTree::levelOrder_I(EventLog* log) {
	if (root == nullptr) return nullptr;
	treeQueue* q = new treeQueue;
	List* temp = nullptr;
	List* curr_t = temp;
	TreeNode* curr = root;
	q->enqueue(curr);
	while (!q->isEmpty()) {
		curr = q->peek();
		if (log != nullptr) log->add(EventTypes::VISIT, curr->id);
		q->dequeue();
		if (temp != nullptr) {
			curr_t->next = new List(curr->id, curr->val);
			curr_t = curr_t->next;
		}
		if (temp == nullptr) {
			temp = new List(curr->id, curr->val);
			curr_t = temp;
		}
		if (curr->left != nullptr) q->enqueue(curr->left);
		if (curr->right != nullptr) q->enqueue(curr->right);
	}
	delete q;
	return temp;
}

void BinaryTree::destroyTree(TreeNode* node)
{
	if (node == nullptr)
		return;

	destroyTree(node->left);
	destroyTree(node->right);

	delete node;
}

BinaryTree::~BinaryTree()
{
	destroyTree(root);
}