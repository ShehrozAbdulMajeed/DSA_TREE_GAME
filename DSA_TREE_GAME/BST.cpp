#include "BST.h"

BST::BST() : BinaryTree()
{
    nextID = 1;
}


bool BST::insert(int key, EventLog* log)
{
    TreeNode* current = root;
    TreeNode* parent = nullptr;

    while (current != nullptr)
    {
        parent = current;

        if (log != nullptr)
        {
            int result = 0;

            if (key < current->val)
                result = -1;
            else if (key > current->val)
                result = 1;

            log->add(EventTypes::COMPARE, current->id, key, result);
        }

        if (key == current->val)
        {
            if (log != nullptr)
                log->add(EventTypes::DONE);

            return false;
        }
        else if (key < current->val)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    TreeNode* newNode = new TreeNode(nextID++, key);

    if (parent == nullptr)
    {
        root = newNode;
    }
    else if (key < parent->val)
    {
        parent->left = newNode;
    }
    else
    {
        parent->right = newNode;
    }

    if (log != nullptr)
    {
        int parentID = 0;
        int side = 0;

        if (parent != nullptr)
        {
            parentID = parent->id;

            if (key < parent->val)
                side = 1;  
            else
                side = 2; 
        }

        log->add(EventTypes::INSERT, newNode->id, parentID, side);
        log->add(EventTypes::DONE);
    }

    return true;
}

TreeNode* BST::searchHelper(TreeNode* node, int key)
{
    if (node == nullptr)
        return nullptr;

    if (node->val == key)
        return node;

    if (key < node->val)
        return searchHelper(node->left, key);

    return searchHelper(node->right, key);
}


TreeNode* BST::search(int key, EventLog* log)
{
    TreeNode* current = root;

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
            current = current->left;
        else
            current = current->right;
    }

    if (log != nullptr)
        log->add(EventTypes::DONE);

    return nullptr;
}


TreeNode* BST::findMinHelper(TreeNode* node)
{
    if (node == nullptr)
        return nullptr;

    while (node->left != nullptr)
    {
        node = node->left;
    }

    return node;
}

TreeNode* BST::findMin()
{
    return findMinHelper(root);
}


TreeNode* BST::findMaxHelper(TreeNode* node)
{
    if (node == nullptr)
        return nullptr;

    while (node->right != nullptr)
    {
        node = node->right;
    }

    return node;
}


TreeNode* BST::findMax()
{
    return findMaxHelper(root);
}


TreeNode* BST::successor(int key)
{
    TreeNode* current = root;
    TreeNode* target = nullptr;

    while (current != nullptr)
    {
        if (current->val == key)
        {
            target = current;
            break;
        }
        else if (key < current->val)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    if (target == nullptr)
        return nullptr;

    // Case 1: Target has a right subtree
    if (target->right != nullptr)
        return findMinHelper(target->right);

    // Case 2: No right subtree
    TreeNode* successorNode = nullptr;
    current = root;

    while (current != nullptr)
    {
        if (key < current->val)
        {
            successorNode = current;
            current = current->left;
        }
        else if (key > current->val)
        {
            current = current->right;
        }
        else
        {
            break;
        }
    }

    return successorNode;
}


TreeNode* BST::predecessor(int key)
{
    TreeNode* current = root;
    TreeNode* target = nullptr;

    while (current != nullptr)
    {
        if (current->val == key)
        {
            target = current;
            break;
        }
        else if (key < current->val)
        {
            current = current->left;
        }
        else
        {
            current = current->right;
        }
    }

    if (target == nullptr)
        return nullptr;

    // Case 1: Target has a left subtree
    if (target->left != nullptr)
        return findMaxHelper(target->left);

    // Case 2: No left subtree
    TreeNode* predecessorNode = nullptr;
    current = root;

    while (current != nullptr)
    {
        if (key > current->val)
        {
            predecessorNode = current;
            current = current->right;
        }
        else if (key < current->val)
        {
            current = current->left;
        }
        else
        {
            break;
        }
    }

    return predecessorNode;
}


bool BST::removeHelper(TreeNode*& node, int key, EventLog* log)
{
    if (node == nullptr)
        return false;

    if (log != nullptr)
    {
        int result = 0;

        if (key < node->val)
            result = -1;
        else if (key > node->val)
            result = 1;

        log->add(EventTypes::COMPARE, node->id, key, result);
    }

    if (key < node->val)
    {
        return removeHelper(node->left, key, log);
    }

    else if (key > node->val)
    {
        return removeHelper(node->right, key, log);
    }


    // Case 1 and Case 2: No left child
    if (node->left == nullptr)
    {
        TreeNode* temp = node;

        if (log != nullptr)
            log->add(EventTypes::UNLINK, temp->id, 0, temp->val);

        node = node->right;
        delete temp;

        return true;
    }

    // Case 2: No right child
    else if (node->right == nullptr)
    {
        TreeNode* temp = node;

        if (log != nullptr)
            log->add(EventTypes::UNLINK, temp->id, 0, temp->val);

        node = node->left;
        delete temp;

        return true;
    }

    // Case 3: Node has two children

    TreeNode* successorNode = findMinHelper(node->right);

    if (log != nullptr)
    {
        log->add(EventTypes::SWAP_VALUE, node->id, successorNode->id, successorNode->val);
    }
    
    node->val = successorNode->val;

    return removeHelper(node->right, successorNode->val, log);
}


bool BST::remove(int key, EventLog* log)
{
    bool result = removeHelper(root, key, log);

    if (log != nullptr)
        log->add(EventTypes::DONE);

    return result;
}


void BST::rangeQueryHelper(TreeNode* node, int lo, int hi,
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

        rangeQueryHelper(node->right, lo, hi, tail, log);
    }

    // Case 2: Current value is larger than the range
    else if (node->val > hi)
    {
        if (node->right != nullptr && log != nullptr)
        {
            log->add(EventTypes::HIGHLIGHT_SUBTREE, node->right->id, 1);
        }

        rangeQueryHelper(node->left, lo, hi, tail, log);
    }

    // Case 3: Current value is inside the range
    else
    {
        rangeQueryHelper(node->left, lo, hi, tail, log);

        if (log != nullptr)
        {
            log->add(EventTypes::HIGHLIGHT_SUBTREE, node->id, 2);
        }

        tail->next = new List(node->id, node->val);
        tail = tail->next;

        rangeQueryHelper(node->right, lo, hi, tail, log);
    }
}


List* BST::rangeQuery(int lo, int hi, EventLog* log)
{
    List dummy;
    dummy.next = nullptr;

    List* tail = &dummy;

    if (lo <= hi)
    {
        rangeQueryHelper(root, lo, hi, tail, log);
    }

    if (log != nullptr)
        log->add(EventTypes::DONE);

    return dummy.next;
}


TreeNode* BST::kthSmallestHelper(TreeNode* node, int k, int& count)
{
    if (node == nullptr)
        return nullptr;

    TreeNode* result = kthSmallestHelper(node->left, k, count);

    if (result != nullptr)
        return result;

    count++;

    if (count == k)
        return node;

    return kthSmallestHelper(node->right, k, count);
}


TreeNode* BST::kthSmallest(int k)
{
    if (k <= 0)
        return nullptr;

    int count = 0;

    return kthSmallestHelper(root, k, count);
}

TreeNode* BST::lcaBST(int a, int b)
{
    if (search(a) == nullptr || search(b) == nullptr)
        return nullptr;

    TreeNode* current = root;

    while (current != nullptr)
    {
        if (a < current->val && b < current->val) current = current->left;

        else if (a > current->val && b > current->val) current = current->right;

        else return current;
    }

    return nullptr;
}







