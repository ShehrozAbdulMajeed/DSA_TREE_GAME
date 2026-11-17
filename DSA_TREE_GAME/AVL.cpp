#include"AVL.h"

int AVL::getHeight(AVLNode* node) {
	if(node != nullptr) return node->height;
	return 0;
}

int AVL::getSize(AVLNode* node) {
	if(node != nullptr) return node->size;
	return 0;
}

void AVL::updateHeight(AVLNode* node) {
if(node != nullptr) node->height = 1+max(getHeight(static_cast<AVLNode*>(node->left)),getHeight(static_cast<AVLNode*>(node->right)));
}

void AVL::updateSize(AVLNode* node) {
	if(node != nullptr) node->size = 1+getSize(static_cast<AVLNode*>(node->left))+getSize(static_cast<AVLNode*>(node->right));
}

int AVL::balanceFactor(AVLNode* node) {
	 return getHeight(static_cast<AVLNode*>(node->left))-getHeight(static_cast<AVLNode*>(node->right));
}

AVLNode* AVL::rotateRight(AVLNode* y) {
	AVLNode* x = static_cast<AVLNode*>(y->left);
	AVLNode* T2 = static_cast<AVLNode*>(x->right);
	x->right = y;
	y->left = T2;
	updateHeight(y);
	updateSize(y);

	updateHeight(x);
	updateSize(x);

	return x;
}

AVLNode* AVL::rotateLeft(AVLNode* y) {
	AVLNode* x = static_cast<AVLNode*>(y->right);
	AVLNode* T2 = static_cast<AVLNode*>(x->left);
	x->left = y;
	y->right = T2;
	updateHeight(y);
	updateSize(y);

	updateHeight(x);
	updateSize(x);

	return x;
}

AVLNode* AVL::insertHelper(AVLNode* node, int key, int payload, EventLog* log, bool& check) {
    if (node == nullptr) {
        AVLNode* temp = new AVLNode(nextID++, key, payload);
        check = true;
        if (log)
            log->add(EventTypes::INSERT, temp->id, 0, 0);

        return temp;
    }

    if (key < node->key) {
        node->left = insertHelper(
            static_cast<AVLNode*>(node->left), key, payload, log
        ,check);
    }
    else if (key > node->key) {
        node->right = insertHelper(
            static_cast<AVLNode*>(node->right), key, payload, log
        ,check);
    }
    else {
        return node; 
    }
return rebalance(node);
}

bool AVL::insert(int key, int payload, EventLog* log) {
bool check = false;
	AVLNode* oldRoot = static_cast<AVLNode*>(root);

	AVLNode* newRoot = insertHelper(oldRoot, key, payload, log,check);
   root = newRoot; 
	return check;
}

AVLNode* AVL::findMinHelper(AVLNode* node)
{
    if (node == nullptr)
        return nullptr;

    while (node->left != nullptr)
    {
        node = static_cast<AVLNode*>(node->left);
    }

    return node;
}

bool AVL::removeHelper(TreeNode*& node, int key, EventLog* log)
{
    if (node == nullptr)
        return false;

    AVLNode* current = static_cast<AVLNode*>(node);

    if (log != nullptr)
    {
        int result = 0;
        if (key < current->key) result = -1;
        else if (key > current->key) result = 1;

        log->add(EventTypes::COMPARE, node->id, key, result);
    }

    bool removed = false;

    if (key < current->key)
    {
        removed = removeHelper(node->left, key, log);
    }
    else if (key > current->key)
    {
        removed = removeHelper(node->right, key, log);
    }
    else
    {
        if (node->left == nullptr)
        {
            TreeNode* oldNode = node;

            if (log != nullptr)
                log->add(EventTypes::UNLINK, oldNode->id, 0, current->key);

            node = node->right;
            delete oldNode;
            return true;
        }

        else if (node->right == nullptr)
        {
            TreeNode* oldNode = node;

            if (log != nullptr)
                log->add(EventTypes::UNLINK, oldNode->id, 0, current->key);

            node = node->left;
            delete oldNode;
            return true;
        }

        else
        {
            AVLNode* successorNode =
                findMinHelper(static_cast<AVLNode*>(node->right));

            int successorKey = successorNode->key;
            int successorPayload = successorNode->payload;

            if (log != nullptr)
            {
                log->add(EventTypes::SWAP_VALUE,
                    node->id, successorNode->id, successorKey);
            }

            current->key = successorKey;
            current->payload = successorPayload;

            removed = removeHelper(node->right, successorKey, log);
        }
    }

    if (!removed)
        return false;

    if (node == nullptr)
        return true;

    current = static_cast<AVLNode*>(node);

   node = rebalance(static_cast<AVLNode*>(node));
    return true;
}

bool AVL::remove(int key, EventLog* log)
{
    bool result = removeHelper(root, key, log);
    if (log != nullptr)
        log->add(EventTypes::DONE);
    return result;
}

AVLNode* AVL::kthHelper(AVLNode* node, int k)
{
    if (node == nullptr || k <= 0 || k > getSize(node))
        return nullptr;

    int leftSize = getSize(static_cast<AVLNode*>(node->left));

    if (k == leftSize + 1)
        return node;

    if (k <= leftSize)
        return kthHelper(static_cast<AVLNode*>(node->left), k);

    return kthHelper(
        static_cast<AVLNode*>(node->right),
        k - leftSize - 1
    );
}

AVLNode* AVL::kth(int k)
{
    return kthHelper(static_cast<AVLNode*>(root), k);
}

int AVL::rankHelper(AVLNode* node, int key)
{
    if (node == nullptr)
        return 0;

    if (key < node->key)
        return rankHelper(static_cast<AVLNode*>(node->left), key);

    int leftSize = getSize(static_cast<AVLNode*>(node->left));

    if (key == node->key)
        return leftSize + 1;

    return leftSize + 1 +
        rankHelper(static_cast<AVLNode*>(node->right), key);
}

int AVL::rank(int key)
{
    return rankHelper(static_cast<AVLNode*>(root), key);
}

int AVL::countInRange(int lo, int hi)
{
    if (lo > hi)
        return 0;

    return rank(hi) - rank(lo - 1);
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

    if (abs(balance) > 1)
        return false;

    if (node->height != actualHeight || node->size != actualSize)
        return false;

    return isBalancedHelper(static_cast<AVLNode*>(node->left))
        && isBalancedHelper(static_cast<AVLNode*>(node->right));
}

bool AVL::isBalanced()
{
    return isBalancedHelper(static_cast<AVLNode*>(root));
}

AVLNode* AVL::rebalance(AVLNode* node)
{
    if (node == nullptr)
        return nullptr;

    updateHeight(node);
    updateSize(node);

    int balance = balanceFactor(node);

    if (balance > 1 &&
        balanceFactor(static_cast<AVLNode*>(node->left)) >= 0)
        return rotateRight(node);

    if (balance > 1 &&
        balanceFactor(static_cast<AVLNode*>(node->left)) < 0)
    {
        node->left = rotateLeft(
            static_cast<AVLNode*>(node->left)
        );
        return rotateRight(node);
    }

    if (balance < -1 &&
        balanceFactor(static_cast<AVLNode*>(node->right)) <= 0)
        return rotateLeft(node);

    if (balance < -1 &&
        balanceFactor(static_cast<AVLNode*>(node->right)) > 0)
    {
        node->right = rotateRight(
            static_cast<AVLNode*>(node->right)
        );
        return rotateLeft(node);
    }

    return node;
}