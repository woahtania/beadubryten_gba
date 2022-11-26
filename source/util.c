#include "util.h"

int clamp(int num, int min, int max)
{
    if (num <= min)
        return min;
    if (num >= max)
        return max;
    return num;
}