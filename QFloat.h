#ifndef _QFLOAT_H_
#define _QFLOAT_H_
#include <stdint.h>
#include <string.h>

#define K ((1 << 15) - 1)

struct QFloat {
    uint8_t val[14];
    int16_t se;  // sign + exponent
    QFloat();
};

QFloat Dec2QFloat(const char *);
QFloat QFloat2Dec(const char *);

#endif
