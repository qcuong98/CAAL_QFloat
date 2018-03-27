#include <stdlib.h>
#include "QFloat.h"
#include "number/number.h"

#include <iostream>
using namespace std;

const int16_t BIAS = 0b0011111111111111;

char *QFloat2Dec(const QFloat &q) {
#define RETURN_STR(_s)                           \
    {                                            \
        const char *s = _s;                      \
        int len       = strlen(s);               \
        char *r       = (char *)malloc(len + 1); \
        memcpy(r, s, len + 1);                   \
        return r;                                \
    }
    if (IsNaN(q)) RETURN_STR("NaN");
    if (IsInf(q)) {
        if (q.se < 0) RETURN_STR("-Inf");
        RETURN_STR("+Inf");
    }

#define RETURN(_X)                    \
    {                                 \
        Number _tmp = (_X);           \
        return _tmp.chop(4).to_str(); \
    }
    const int BIT_COUNT = sizeof(QFloat::val) * 8;
    int exponent        = q.se & 0b0111111111111111;
    if (exponent == 0) {
        // denormalized or 0
        // Fuck denormalization
        RETURN(Number("0"));
    }

    exponent -= BIAS;
    const int THRESHOLD = 100;
    if (abs(exponent) <= THRESHOLD) {
        Number res(0ll);
        Number pow = TWO ^ Number(-BIT_COUNT + exponent);
        for (int i = 0; i < BIT_COUNT; i++) {
            if ((q.val[i >> 3] >> (i & 7)) & 1) {
                res = res + pow;
            }
            pow = pow * TWO;
        }
        res = res + pow;

        if (q.se < 0)
            res = -res;

        RETURN(res);
    } else {
        Number res(0ll);
        Number pow = ONE;
        res        = res + pow;
        for (int i = -1; i >= -BIT_COUNT; i--) {
            pow   = pow / TWO;
            int j = BIT_COUNT + i;
            if ((q.val[j >> 3] >> (j & 7)) & 1)
                res = res + pow;
        }
        Number exp = Number(exponent) * LOG2;
        Number E   = exp.floor();
        res        = res * (exp.fraction() * LN10).exp();
        
        char *s = res.chop(4).to_str();
        int len = strlen(s);
        s = (char*)realloc(s, len + 10);
        char *exp_s = E.to_str();
        s[len] = 'e';
        strcpy(s + len + 1, exp_s);
        free(exp_s);
        return s;
    }
}
