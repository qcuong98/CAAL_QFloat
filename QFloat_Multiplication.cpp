#include "QFloat.h"

QFloat operator * (const QFloat& a, const QFloat& b) {
        /* handle denormalized floating point */
        QFloat c;

        int x = (a.se & 1) ^ (b.se & 1);
        int sign = x ? 0 : 1;
        
        // too lazy to code
        return c;
}