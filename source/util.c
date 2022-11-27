#include "util.h"

int clamp(int num, int min, int max)
{
    if (num <= min)
        return min;
    if (num >= max)
        return max;
    return num;
}

int lerp(int num, int target, int delta)
{
    if (num < target)
        num += delta;
    if (num > target)
        num -= delta;

    if (num > target - (delta * 2) && num < target + (delta * 2))
        return target;
    return num;
}