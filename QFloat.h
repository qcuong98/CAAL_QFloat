#ifndef _QFLOAT_H_
#define _QFLOAT_H_
#include <stdint.h>
#include <string.h>

#define K ((1 << 15) - 1)
#define MANT 14

struct QFloat {
    uint8_t val[MANT];
    uint16_t se;  // sign + exponent, sign is MSB
    QFloat();
};

QFloat Dec2QFloat(const char *);
char *QFloat2Dec(const QFloat&); //remeber to free

QFloat operator +(const QFloat &, const QFloat&);
#endif
