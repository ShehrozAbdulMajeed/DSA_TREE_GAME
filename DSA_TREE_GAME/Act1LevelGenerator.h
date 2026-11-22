#pragma once

#include "BinaryTree.h"
#include "SeededRandom.h"

class Act1LevelGenerator
{
public:
    static TreeNode* generate(
        unsigned int seed,
        int minimumNodes = 6,
        int maximumNodes = 15)
    {
        if (minimumNodes < 1)
            minimumNodes = 1;

        if (maximumNodes < minimumNodes)
            maximumNodes = minimumNodes;

        SeededRandom random(seed);

        int count = random.range(minimumNodes, maximumNodes);

        int* values = new int[count];

        for (int i = 0; i < count; i++)
            values[i] = 10 + i;

        random.shuffle(values, count);

        TreeNode* root = nullptr;

        // Each entry points to an available child position.
        TreeNode*** slots = new TreeNode * *[2 * count + 1];

        int slotCount = 1;
        slots[0] = &root;

        for (int i = 0; i < count; i++)
        {
            // Choose one currently available child position.
            int chosen = random.range(0, slotCount - 1);

            TreeNode** destination = slots[chosen];

            // Remove the chosen position from the available slots.
            slots[chosen] = slots[slotCount - 1];
            slotCount--;

            TreeNode* node = new TreeNode(i + 1, values[i]);
            *destination = node;

            // Both child positions become available.
            slots[slotCount++] = &node->left;
            slots[slotCount++] = &node->right;
        }

        delete[] slots;
        delete[] values;

        return root;
    }

    static void destroyTree(TreeNode* node)
    {
        if (node == nullptr)
            return;

        destroyTree(node->left);
        destroyTree(node->right);

        delete node;
    }

    static TreeNode* findLCA(
        TreeNode* root,
        int firstID,
        int secondID)
    {
        if (root == nullptr)
            return nullptr;

        if (root->id == firstID || root->id == secondID)
            return root;

        TreeNode* left = findLCA(root->left, firstID, secondID);
        TreeNode* right = findLCA(root->right, firstID, secondID);

        if (left != nullptr && right != nullptr)
            return root;

        return left != nullptr ? left : right;
    }
};