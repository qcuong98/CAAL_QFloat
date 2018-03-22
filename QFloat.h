#ifndef _QFLOAT_H_
#define _QFLOAT_H_
#include <stdint.h>
#include <string.h>

struct QFloat {
    int16_t se;  // sign + exponent
    uint8_t val[14];
    QFloat();
};

QFloat Dec2QFloat(const char *);
QFloat QFloat2Dec(const char *);

#endif
