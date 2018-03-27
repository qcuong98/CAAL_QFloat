#include "QFloat.h"
#include <stdlib.h>

char *QFloat2Bin(const QFloat &q) {
    const int *data = (int *)&q;
#define BIT(i) ('0' | ((data[(i) >> 5] >> (i & 31)) & 1))
    const int LEN = 130;
    char *res = (char *)malloc(LEN + 1);
    res[LEN] = 0;
    res[1] = res[17] = '_';
    res[0] = BIT(127);
    int i = 126;
    for (int c = 0; c < NUMBER_EXPONENT_BITS; c++) {
        res[128 - i] = BIT(i);
        i--;
    }
    for (int c = 0; c < NUMBER_SIGNIFICAND_BYTES * 8; c++) {
        res[129 - i] = BIT(i);
        i--;
    }
    return res;
}
