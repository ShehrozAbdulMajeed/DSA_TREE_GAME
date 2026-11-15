#include <iostream>
#include "BST.h"

using namespace std;

int main()
{
    BST tree;

    // Test insertion
    cout << "Inserting values:\n";

    cout << tree.insert(50) << endl;
    cout << tree.insert(30) << endl;
    cout << tree.insert(70) << endl;
    cout << tree.insert(20) << endl;
    cout << tree.insert(40) << endl;
    cout << tree.insert(60) << endl;
    cout << tree.insert(80) << endl;

    // Test duplicate
    cout << "\nInserting duplicate 30:\n";
    cout << tree.insert(30) << endl;

    // Test search
    cout << "\nSearching for 40:\n";

    TreeNode* found = tree.search(40);

    if (found != nullptr)
        cout << "Found: " << found->val << endl;
    else
        cout << "Not found\n";

    // Test unsuccessful search
    cout << "\nSearching for 100:\n";

    found = tree.search(100);

    if (found != nullptr)
        cout << "Found: " << found->val << endl;
    else
        cout << "Not found\n";

    // Test minimum
    cout << "\nMinimum value:\n";

    TreeNode* minimum = tree.findMin();

    if (minimum != nullptr)
        cout << minimum->val << endl;

    // Test maximum
    cout << "\nMaximum value:\n";

    TreeNode* maximum = tree.findMax();

    if (maximum != nullptr)
        cout << maximum->val << endl;

    // Test successor
    cout << "\nSuccessor of 30:\n";

    TreeNode* next = tree.successor(30);

    if (next != nullptr)
        cout << next->val << endl;
    else
        cout << "No successor\n";


    // Test predecessor
    cout << "\nPredecessor of 30:\n";

    TreeNode* previous = tree.predecessor(30);

    if (previous != nullptr)
        cout << previous->val << endl;
    else
        cout << "No predecessor\n";


    // Test deletion of a leaf node
    cout << "\nDeleting 20 (leaf node):\n";
    cout << tree.remove(20) << endl;


    // Test deletion of a node with one child
    tree.insert(25);

    cout << "\nDeleting 30 (one child):\n";
    cout << tree.remove(30) << endl;


    // Test deletion of a node with two children
    cout << "\nDeleting 70 (two children):\n";
    cout << tree.remove(70) << endl;

    // Test range query
    cout << "\nRange query (35 to 65):\n";

    List* result = tree.rangeQuery(35, 65);

    List* current = result;

    while (current != nullptr)
    {
        cout << current->val << " ";
        current = current->next;
    }

    cout << endl;

    // Free the returned linked list
    current = result;

    while (current != nullptr)
    {
        List* temp = current;
        current = current->next;
        delete temp;
    }

    // Test kth smallest
    cout << "\n3rd smallest value:\n";

    TreeNode* kth = tree.kthSmallest(3);

    if (kth != nullptr)
        cout << kth->val << endl;
    else
        cout << "Invalid k\n";

    // Test Lowest Common Ancestor
    cout << "\nLCA of 40 and 60:\n";

    TreeNode* ancestor = tree.lcaBST(40, 60);

    if (ancestor != nullptr)
        cout << ancestor->val << endl;
    else
        cout << "LCA not found\n";

    return 0;
}