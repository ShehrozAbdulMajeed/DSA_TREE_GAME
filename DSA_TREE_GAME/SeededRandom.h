#pragma once

class SeededRandom
{
private:
    unsigned int state;

public:
    explicit SeededRandom(unsigned int seed = 1)
    {
        state = seed;

        if (state == 0)
            state = 0xA341316C;
    }

    unsigned int next()
    {
        unsigned int x = state;

        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;

        state = x;
        return state;
    }

    int range(int minimum, int maximum)
    {
        if (minimum > maximum)
        {
            int temp = minimum;
            minimum = maximum;
            maximum = temp;
        }

        unsigned int span =
            static_cast<unsigned int>(maximum - minimum) + 1;

        return minimum + static_cast<int>(next() % span);
    }

    bool chance(int numerator, int denominator)
    {
        if (denominator <= 0 || numerator <= 0)
            return false;

        if (numerator >= denominator)
            return true;

        return range(1, denominator) <= numerator;
    }

    void shuffle(int* values, int count)
    {
        if (values == nullptr || count <= 1)
            return;

        for (int i = count - 1; i > 0; i--)
        {
            int j = range(0, i);

            int temp = values[i];
            values[i] = values[j];
            values[j] = temp;
        }
    }

    static unsigned int deriveSeed(
        unsigned int campaignSeed,
        unsigned int act,
        unsigned int level)
    {
        unsigned int x = campaignSeed;

        x ^= act * 0x9E3779B9;
        x ^= level * 0x85EBCA6B;

        x ^= x >> 16;
        x *= 0x7FEB352D;
        x ^= x >> 15;
        x *= 0x846CA68B;
        x ^= x >> 16;

        if (x == 0)
            x = 0xA341316C;

        return x;
    }
};