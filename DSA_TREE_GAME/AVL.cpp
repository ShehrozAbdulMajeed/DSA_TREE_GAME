#include"AVL.h"

int AVL::getHeight(AVLNode* node) {
	if (node != nullptr) return node->height;
	return 0;
}

int AVL::getSize(AVLNode* node) {
	if (node != nullptr) return node->size;
	return 0;
}

void AVL::updateHeight(AVLNode* node, EventLog* log) {
	if (node == nullptr) return;
	int old = node->height;
	node->height = 1 + max(getHeight(static_cast<AVLNode*>(node->left)), getHeight(static_cast<AVLNode*>(node->right)));
	if (log != nullptr && old != node->height)
		log->add(EventTypes::UPDATE_HEIGHT, node->id, old, node->height);
}

void AVL::updateSize(AVLNode* node, EventLog* log) {
	if (node == nullptr) return;
	int old = node->size;
	node->size = 1 + getSize(static_cast<AVLNode*>(node->left)) + getSize(static_cast<AVLNode*>(node->right));
	if (log != nullptr && old != node->size)
		log->add(EventTypes::UPDATE_SIZE, node->id, old, node->size);
}

int AVL::balanceFactor(AVLNode* node) {
	if (node == nullptr) return 0;
	return getHeight(static_cast<AVLNode*>(node->left)) - getHeight(static_cast<AVLNode*>(node->right));
}

AVLNode* AVL::rotateRight(AVLNode* y, EventLog* log) {
	AVLNode* x = static_cast<AVLNode*>(y->left);
	AVLNode* T2 = static_cast<AVLNode*>(x->right);
	x->right = y;
	y->left = T2;
	rotationCount++;

	updateHeight(y, log);
	updateSize(y, log);
	updateHeight(x, log);
	updateSize(x, log);

	return x;
}

AVLNode* AVL::rotateLeft(AVLNode* y, EventLog* log) {
	AVLNode* x = static_cast<AVLNode*>(y->right);
	AVLNode* T2 = static_cast<AVLNode*>(x->left);
	x->left = y;
	y->right = T2;
	rotationCount++;

	updateHeight(y, log);
	updateSize(y, log);
	updateHeight(x, log);
	updateSize(x, log);

	return x;
}

AVLNode* AVL::rebalance(AVLNode* node, EventLog* log) {
	if (node == nullptr)
		return nullptr;

	updateHeight(node, log);
	updateSize(node, log);

	int bf = balanceFactor(node);
	if (bf >= -1 && bf <= 1)
		return node;

	if (log != nullptr)
		log->add(EventTypes::BALANCE_ALERT, node->id, bf);

	if (bf > 1) {
		AVLNode* l = static_cast<AVLNode*>(node->left);
		if (balanceFactor(l) >= 0) {                       // LL
			if (log != nullptr)
				log->add(EventTypes::ROTATE, node->id, RotationType::LL, l->id);
			return rotateRight(node, log);
		}
		AVLNode* lr = static_cast<AVLNode*>(l->right);     // LR
		if (log != nullptr)
			log->add(EventTypes::ROTATE, node->id, RotationType::LR, lr->id);
		node->left = rotateLeft(l, log);
		return rotateRight(node, log);
	}

	AVLNode* r = static_cast<AVLNode*>(node->right);
	if (balanceFactor(r) <= 0) {                           // RR
		if (log != nullptr)
			log->add(EventTypes::ROTATE, node->id, RotationType::RR, r->id);
		return rotateLeft(node, log);
	}
	AVLNode* rl = static_cast<AVLNode*>(r->left);          // RL
	if (log != nullptr)
		log->add(EventTypes::ROTATE, node->id, RotationType::RL, rl->id);
	node->right = rotateRight(r, log);
	return rotateLeft(node, log);
}

AVLNode* AVL::insertHelper(AVLNode* node, int key, int payload, EventLog* log,
	bool& inserted, int parentId, int side) {
	if (node == nullptr) {
		AVLNode* temp = new AVLNode(nextID++, key, payload);
		inserted = true;
		if (log != nullptr)
			log->add(EventTypes::INSERT, temp->id, parentId, side);
		return temp;
	}

	int result = 0;
	if (key < node->key) result = -1;
	else if (key > node->key) result = 1;

	compareCount++;
	if (log != nullptr)
		log->add(EventTypes::COMPARE, node->id, key, result);

	if (result < 0)
		node->left = insertHelper(static_cast<AVLNode*>(node->left), key, payload, log, inserted, node->id, 1);
	else if (result > 0)
		node->right = insertHelper(static_cast<AVLNode*>(node->right), key, payload, log, inserted, node->id, 2);
	else
		return node;   // duplicate rejected

	return rebalance(node, log);
}

bool AVL::insert(int key, int payload, EventLog* log) {
	bool inserted = false;
	root = insertHelper(static_cast<AVLNode*>(root), key, payload, log, inserted, 0, 0);
	if (log != nullptr)
		log->add(EventTypes::DONE);
	return inserted;
}

AVLNode* AVL::findMinHelper(AVLNode* node)
{
	if (node == nullptr)
		return nullptr;

	while (node->left != nullptr)
		node = static_cast<AVLNode*>(node->left);

	return node;
}

bool AVL::removeHelper(TreeNode*& node, int key, EventLog* log, int parentId, int side)
{
	if (node == nullptr)
		return false;

	AVLNode* current = static_cast<AVLNode*>(node);

	int result = 0;
	if (key < current->key) result = -1;
	else if (key > current->key) result = 1;

	compareCount++;
	if (log != nullptr)
		log->add(EventTypes::COMPARE, current->id, key, result);

	if (result < 0)
	{
		if (!removeHelper(node->left, key, log, current->id, 1))
			return false;
	}
	else if (result > 0)
	{
		if (!removeHelper(node->right, key, log, current->id, 2))
			return false;
	}
	else
	{
		// Leaf or single child: bypass. Ancestors rebalance on the way back up.
		if (node->left == nullptr || node->right == nullptr)
		{
			TreeNode* oldNode = node;

			if (log != nullptr)
				log->add(EventTypes::UNLINK, oldNode->id, parentId, side);

			node = (node->left != nullptr) ? node->left : node->right;
			delete oldNode;
			return true;
		}

		// Two children: copy inorder successor here, then delete the successor.
		AVLNode* successor = findMinHelper(static_cast<AVLNode*>(node->right));
		int successorKey = successor->key;

		if (log != nullptr)
			log->add(EventTypes::SWAP_VALUE, current->id, successor->id, successorKey);

		current->key = successorKey;
		current->val = successorKey;          // val and key must stay equal
		current->payload = successor->payload;

		if (!removeHelper(node->right, successorKey, log, current->id, 2))
			return false;
	}

	node = rebalance(static_cast<AVLNode*>(node), log);
	return true;
}

bool AVL::remove(int key, EventLog* log)
{
	bool result = removeHelper(root, key, log, 0, 0);
	if (log != nullptr)
		log->add(EventTypes::DONE);
	return result;
}

AVLNode* AVL::kthHelper(AVLNode* node, int k, EventLog* log)
{
	if (node == nullptr || k <= 0 || k > getSize(node))
		return nullptr;

	if (log != nullptr)
		log->add(EventTypes::VISIT, node->id);

	int leftSize = getSize(static_cast<AVLNode*>(node->left));

	if (k == leftSize + 1)
		return node;

	if (k <= leftSize)
		return kthHelper(static_cast<AVLNode*>(node->left), k, log);

	return kthHelper(static_cast<AVLNode*>(node->right), k - leftSize - 1, log);
}

AVLNode* AVL::kth(int k, EventLog* log)
{
	AVLNode* result = kthHelper(static_cast<AVLNode*>(root), k, log);
	if (log != nullptr)
		log->add(EventTypes::DONE);
	return result;
}

int AVL::rankHelper(AVLNode* node, int key, EventLog* log)
{
	if (node == nullptr)
		return 0;

	if (log != nullptr)
		log->add(EventTypes::VISIT, node->id);

	if (key < node->key)
		return rankHelper(static_cast<AVLNode*>(node->left), key, log);

	int leftSize = getSize(static_cast<AVLNode*>(node->left));

	if (key == node->key)
		return leftSize + 1;

	return leftSize + 1 + rankHelper(static_cast<AVLNode*>(node->right), key, log);
}

// Number of keys <= key
int AVL::rank(int key, EventLog* log)
{
	int result = rankHelper(static_cast<AVLNode*>(root), key, log);
	if (log != nullptr)
		log->add(EventTypes::DONE);
	return result;
}

int AVL::countInRange(int lo, int hi, EventLog* log)
{
	int result = 0;
	if (lo <= hi)
		result = rankHelper(static_cast<AVLNode*>(root), hi, log)
		- rankHelper(static_cast<AVLNode*>(root), lo - 1, log);
	if (log != nullptr)
		log->add(EventTypes::DONE);
	return result;
}

bool AVL::isBalancedHelper(AVLNode* node)
{
	if (node == nullptr)
		return true;

	int leftHeight = getHeight(static_cast<AVLNode*>(node->left));
	int rightHeight = getHeight(static_cast<AVLNode*>(node->right));

	int actualHeight = 1 + max(leftHeight, rightHeight);

	int actualSize = 1
		+ getSize(static_cast<AVLNode*>(node->left))
		+ getSize(static_cast<AVLNode*>(node->right));

	int balance = leftHeight - rightHeight;

	if (balance > 1 || balance < -1)
		return false;

	if (node->height != actualHeight || node->size != actualSize)
		return false;

	return isBalancedHelper(static_cast<AVLNode*>(node->left))
		&& isBalancedHelper(static_cast<AVLNode*>(node->right));
}

void AVL::recomputeHelper(AVLNode* node)
{
	if (node == nullptr) return;
	recomputeHelper(static_cast<AVLNode*>(node->left));
	recomputeHelper(static_cast<AVLNode*>(node->right));
	updateHeight(node, nullptr);
	updateSize(node, nullptr);
}

void AVL::recompute()
{
	recomputeHelper(static_cast<AVLNode*>(root));
}

bool AVL::isBalanced()
{
	return isBalancedHelper(static_cast<AVLNode*>(root));
}