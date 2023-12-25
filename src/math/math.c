#include "math.h"

int
ceiling(float x)
{
    int i = (int)x;
    if (x > i) {
        return i + 1;
    }
    return i;
}
