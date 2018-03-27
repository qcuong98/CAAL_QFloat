#ifndef _QFLOAT_H_
#define _QFLOAT_H_
#include <stdint.h>
#include <string.h>

#define NUMBER_EXPONENT_BITS 15
#define NUMBER_SIGNIFICAND_BYTES 14
#define K ((1 << NUMBER_EXPONENT_BITS) - 1)

struct QFloat {
    int16_t se;  // sign + exponent, sign is MSB
    uint8_t val[NUMBER_SIGNIFICAND_BYTES];
    QFloat();
    static const QFloat Inf;
    static const QFloat NaN;
};


QFloat Dec2QFloat(const char *);
char *QFloat2Dec(const QFloat&); //remeber to free

QFloat operator +(const QFloat &, const QFloat&);

QFloat operator -(const QFloat &);
QFloat operator -(const QFloat &, const QFloat&);

bool IsInf(const QFloat &);
bool IsNaN(const QFloat &);

#endif
