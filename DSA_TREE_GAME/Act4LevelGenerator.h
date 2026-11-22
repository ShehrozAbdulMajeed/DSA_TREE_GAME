#pragma once

#include "SeededRandom.h"

class Act4LevelGenerator
{
private:
    static void swapValues(int& a, int& b)
    {
        int temp = a;
        a = b;
        b = temp;
    }

    static void buildHeap(int* values, int count)
    {
        for (int i = count / 2 - 1; i >= 0; i--)
        {
            int current = i;

            while (true)
            {
                int largest = current;
                int left = 2 * current + 1;
                int right = 2 * current + 2;

                if (left < count &&
                    values[left] > values[largest])
                    largest = left;

                if (right < count &&
                    values[right] > values[largest])
                    largest = right;

                if (largest == current)
                    break;

                swapValues(values[current], values[largest]);
                current = largest;
            }
        }
    }

public:
    static void generate(
        int mission,
        unsigned int seed,
        int* output,
        int& count,
        int& optimalSteps)
    {
        if (output == nullptr)
            return;

        SeededRandom random(seed);

        if (mission < 1 || mission > 5)
            mission = 1;

        count = 7;
        optimalSteps = 1;

        if (mission == 1 || mission == 2)
        {
            // Distinct values, with non-overlapping ranges.
            for (int i = 0; i < count; i++)
                output[i] = random.range(10, 99) + i * 100;

            buildHeap(output, count);

            if (mission == 1)
            {
                // One root/leaf swap repairs the heap.
                int leaf = random.range(3, count - 1);
                swapValues(output[0], output[leaf]);
                optimalSteps = 1;
            }
            else
            {
                // One sift-up from index 1 repairs the heap.
                output[1] = output[0] + random.range(1, 20);
                optimalSteps = 1;
            }
        }
        else if (mission == 3)
        {
            // Already a valid heap: extract every element.
            for (int i = 0; i < count; i++)
                output[i] = random.range(10, 99) + i * 100;

            buildHeap(output, count);
            optimalSteps = count;
        }
        else if (mission == 4)
        {
            // Duplicate-heavy input.
            for (int i = 0; i < count; i++)
                output[i] = random.range(1, 4) * 10;

            optimalSteps = count;
        }
        else
        {
            // Reverse-sorted input.
            int start = random.range(60, 100);

            for (int i = 0; i < count; i++)
                output[i] = start - i * 5;

            optimalSteps = count;
        }
    }
};