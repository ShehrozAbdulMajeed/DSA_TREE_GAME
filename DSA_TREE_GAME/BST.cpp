#include "BST.h"

BST::BST() : BinaryTree()
{
    nextID = 1;
    compareCount = 0;
}

BSTNode* BST::bstRoot() const { return static_cast<BSTNode*>(root); }

bool BST::insert(int key, EventLog* log)
{
    BSTNode* current = bstRoot();
    BSTNode* parent = nullptr;

    while (current != nullptr)
    {
        parent = current;

        int result = 0;
        if (key < current->val) result = -1;
        else if (key > current->val) result = 1;

        compareCount++;
        if (log != nullptr)
            log->add(EventTypes::COMPARE, current->id, key, result);

        if (result == 0)
        {
            if (log != nullptr)
                log->add(EventTypes::DONE);
            return false;
        }

        if (result < 0)
            current = static_cast<BSTNode*>(current->left);
        else
            current = static_cast<BSTNode*>(current->right);
    }

    BSTNode* newNode = new BSTNode(nextID++, key, 0);

    int parentID = 0;
    int side = 0;   // 0 = root, 1 = left, 2 = right

    if (parent == nullptr)
    {
        root = newNode;
    }
    else if (key < parent->val)
    {
        parent->left = newNode;
        parentID = parent->id;
        side = 1;
    }
    else
    {
        parent->right = newNode;
        parentID = parent->id;
        side = 2;
    }

    if (log != nullptr)
    {
        log->add(EventTypes::INSERT, newNode->id, parentID, side);
        log->add(EventTypes::DONE);
    }

    return true;
}

BSTNode* BST::searchHelper(BSTNode* node, int key)
{
    if (node == nullptr)
        return nullptr;

    if (node->val == key)
        return node;

    if (key < node->val)
        return searchHelper(static_cast<BSTNode*>(node->left), key);

    return searchHelper(static_cast<BSTNode*>(node->right), key);
}


BSTNode* BST::search(int key, EventLog* log)
{
    BSTNode* current = bstRoot();

    while (current != nullptr)
    {
        if (log != nullptr)
        {
            int result = 0;

            if (key < current->val)
                result = -1;
            else if (key > current->val)
                result = 1;

            log->add(EventTypes::COMPARE, current->id, key, result);
        }
        compareCount++;

        if (current->val == key)
        {
            if (log != nullptr)
            {
                log->add(EventTypes::VISIT, current->id);
                log->add(EventTypes::DONE);
            }

            return current;
        }

        if (key < current->val)
            current = static_cast<BSTNode*>(current->left);
        else
            current = static_cast<BSTNode*>(current->right);
    }

    if (log != nullptr)
        log->add(EventTypes::DONE);

    return nullptr;
}


BSTNode* BST::findMinHelper(BSTNode* node)
{
    if (node == nullptr)
        return nullptr;

    while (node->left != nullptr)
    {
        node = static_cast<BSTNode*>(node->left);
    }

    return node;
}

BSTNode* BST::findMin()
{
    return findMinHelper(bstRoot());
}


BSTNode* BST::findMaxHelper(BSTNode* node)
{
    if (node == nullptr)
        return nullptr;

    while (node->right != nullptr)
    {
        node = static_cast<BSTNode*>(node->right);
    }

    return node;
}


BSTNode* BST::findMax()
{
    return findMaxHelper(bstRoot());
}


BSTNode* BST::successor(int key)
{
    BSTNode* current = bstRoot();
    BSTNode* target = nullptr;

    while (current != nullptr)
    {
        if (current->val == key)
        {
            target = current;
            break;
        }
        else if (key < current->val)
        {
            current = static_cast<BSTNode*>(current->left);
        }
        else
        {
            current = static_cast<BSTNode*>(current->right);
        }
    }

    if (target == nullptr)
        return nullptr;

    // Case 1: Target has a right subtree
    if (target->right != nullptr)
        return findMinHelper(static_cast<BSTNode*>(target->right));

    // Case 2: No right subtree
    TreeNode* successorNode = nullptr;
    current = bstRoot();

    while (current != nullptr)
    {
        if (key < current->val)
        {
            successorNode = current;
            current = static_cast<BSTNode*>(current->left);
        }
        else if (key > current->val)
        {
            current = static_cast<BSTNode*>(current->right);
        }
        else
        {
            break;
        }
    }

    return static_cast<BSTNode*>(successorNode);
}


BSTNode* BST::predecessor(int key)
{
    BSTNode* current = bstRoot();
    BSTNode* target = nullptr;

    while (current != nullptr)
    {
        if (current->val == key)
        {
            target = current;
            break;
        }
        else if (key < current->val)
        {
            current = static_cast<BSTNode*>(current->left);
        }
        else
        {
            current = static_cast<BSTNode*>(current->right);
        }
    }

    if (target == nullptr)
        return nullptr;

    // Case 1: Target has a left subtree
    if (target->left != nullptr)
        return findMaxHelper(static_cast<BSTNode*>(target->left));

    // Case 2: No left subtree
    TreeNode* predecessorNode = nullptr;
    current = bstRoot();

    while (current != nullptr)
    {
        if (key > current->val)
        {
            predecessorNode = current;
            current = static_cast<BSTNode*>(current->right);
        }
        else if (key < current->val)
        {
            current = static_cast<BSTNode*>(current->left);
        }
        else
        {
            break;
        }
    }

    return static_cast<BSTNode*>(predecessorNode);
}

bool BST::removeHelper(TreeNode*& node, int key, EventLog* log, int parentId, int side)
{
    if (node == nullptr)
        return false;

    int result = 0;
    if (key < node->val) result = -1;
    else if (key > node->val) result = 1;

    compareCount++;
    if (log != nullptr)
        log->add(EventTypes::COMPARE, node->id, key, result);

    if (result < 0)
        return removeHelper(node->left, key, log, node->id, 1);

    if (result > 0)
        return removeHelper(node->right, key, log, node->id, 2);

    // Case 1 and 2a: no left child (leaf, or only a right child)
    if (node->left == nullptr)
    {
        TreeNode* old = node;

        if (log != nullptr)
            log->add(EventTypes::UNLINK, old->id, parentId, side);

        node = node->right;
        delete old;
        return true;
    }

    // Case 2b: only a left child
    if (node->right == nullptr)
    {
        TreeNode* old = node;

        if (log != nullptr)
            log->add(EventTypes::UNLINK, old->id, parentId, side);

        node = node->left;
        delete old;
        return true;
    }

    // Case 3: two children -> copy inorder successor into this node, delete the successor
    BSTNode* target = static_cast<BSTNode*>(node);
    BSTNode* succ = findMinHelper(static_cast<BSTNode*>(node->right));

    if (log != nullptr)
        log->add(EventTypes::SWAP_VALUE, target->id, succ->id, succ->key);

    target->val = succ->val;
    target->key = succ->key;
    target->payload = succ->payload;

    return removeHelper(node->right, succ->key, log, node->id, 2);
}

bool BST::remove(int key, EventLog* log)
{
    bool result = removeHelper(root, key, log, 0, 0);

    if (log != nullptr)
        log->add(EventTypes::DONE);

    return result;
}


void BST::rangeQueryHelper(BSTNode* node, int lo, int hi,
    List*& tail, EventLog* log)
{
    if (node == nullptr)
        return;

    if (log != nullptr)
        log->add(EventTypes::HIGHLIGHT_SUBTREE, node->id, 0);

    // Case 1: Current value is smaller than the range
    if (node->val < lo)
    {
        if (node->left != nullptr && log != nullptr)
        {
            log->add(EventTypes::HIGHLIGHT_SUBTREE, node->left->id, 1);
        }

        rangeQueryHelper(static_cast<BSTNode*>(node->right), lo, hi, tail, log);
    }

    // Case 2: Current value is larger than the range
    else if (node->val > hi)
    {
        if (node->right != nullptr && log != nullptr)
        {
            log->add(EventTypes::HIGHLIGHT_SUBTREE, node->right->id, 1);
        }

        rangeQueryHelper(static_cast<BSTNode*>(node->left), lo, hi, tail, log);
    }

    // Case 3: Current value is inside the range
    else
    {
        rangeQueryHelper(static_cast<BSTNode*>(node->left), lo, hi, tail, log);

        if (log != nullptr)
        {
            log->add(EventTypes::HIGHLIGHT_SUBTREE, node->id, 2);
        }

        tail->next = new List(node->id, node->val);
        tail = tail->next;

        rangeQueryHelper(static_cast<BSTNode*>(node->right), lo, hi, tail, log);
    }
}


List* BST::rangeQuery(int lo, int hi, EventLog* log)
{
    List dummy;
    dummy.next = nullptr;

    List* tail = &dummy;

    if (lo <= hi)
    {
        rangeQueryHelper(bstRoot(), lo, hi, tail, log);
    }

    if (log != nullptr)
        log->add(EventTypes::DONE);

    return dummy.next;
}


BSTNode* BST::kthSmallestHelper(BSTNode* node, int k, int& count)
{
    if (node == nullptr)
        return nullptr;

    BSTNode* result = kthSmallestHelper(static_cast<BSTNode*>(node->left), k, count);

    if (result != nullptr)
        return result;

    count++;

    if (count == k)
        return node;

    return kthSmallestHelper(static_cast<BSTNode*>(node->right), k, count);
}


BSTNode* BST::kthSmallest(int k)
{
    if (k <= 0)
        return nullptr;

    int count = 0;

    return kthSmallestHelper(bstRoot(), k, count);
}

BSTNode* BST::lcaBST(int a, int b)
{
    if (searchHelper(bstRoot(), a) == nullptr || searchHelper(bstRoot(), b) == nullptr)
        return nullptr;

    BSTNode* current = bstRoot();

    while (current != nullptr)
    {
        if (a < current->val && b < current->val) current = static_cast<BSTNode*>(current->left);

        else if (a > current->val && b > current->val) current = static_cast<BSTNode*>(current->right);

        else return current;
    }

    return nullptr;
}