#ifndef _QFLOAT_H_
#define _QFLOAT_H_
#include <stdint.h>
#include <string.h>

#define NUMBER_EXPONENT_BITS 15
#define NUMBER_SIGNIFICAND_BYTES 14
#define K ((1 << NUMBER_EXPONENT_BITS) - 1)

struct QFloat {
    int16_t se;  // sign + exponent
    uint8_t val[NUMBER_SIGNIFICAND_BYTES];
    QFloat() {
        se = -K;
        memset(val, 0, sizeof(val));
    }
};

QFloat Dec2QFloat(const char *);
QFloat QFloat2Dec(const char *);

#endif
