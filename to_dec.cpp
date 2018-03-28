#include <stdlib.h>
#include "QFloat.h"
#include "number/number.h"

static const Number TEN(10ll);
const int16_t BIAS = 0b0011111111111111;

static char *sign(char *s, int16_t se) {
    if (se >= 0) return s;
    int len = strlen(s);
    s = (char *)realloc(s, len + 2);
    memmove(s + 1, s, len+1);
    s[0] = '-';
    return s;
}

char *QFloat2Dec(const QFloat &Q) {
#define RETURN_STR(_s)                           \
    {                                            \
        const char *s = _s;                      \
        int len       = strlen(s);               \
        char *r       = (char *)malloc(len + 1); \
        memcpy(r, s, len + 1);                   \
        return r;                                \
    }
    if (IsNaN(Q))
        RETURN_STR("NaN");
    if (IsInf(Q)) {
        if (Q.se < 0)
            RETURN_STR("-Inf");
        RETURN_STR("+Inf");
    }

    QFloat q = Q.se < 0? -Q : Q;

    const int BIT_COUNT  = sizeof(QFloat::val) * 8;
    int exponent         = q.se & 0b0111111111111111;
    bool is_denormalized = false;
    if (exponent == 0) {
        // denormalized or 0
        is_denormalized = true;
        bool is_zero    = true;
        for (int i = 0; i < BIT_COUNT; i++) {
            if ((q.val[i >> 3] >> (i & 7)) & 1) {
                is_zero = false;
                break;
            }
        }
        if (is_zero)
            return Number(0ll).to_str();
    }

    exponent -= BIAS;
    const int THRESHOLD = 112;
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

        return sign(res.round().to_str(), Q.se);
    } else {
	++exponent;
        Number res(0ll);
        Number pow = ONE;
        if (!is_denormalized)
            res = res + pow;
        for (int i = -1; i >= -BIT_COUNT; i--) {
            pow   = pow / TWO;
            int j = BIT_COUNT + i;
            if ((q.val[j >> 3] >> (j & 7)) & 1)
                res = res + pow;
        }

        Number exp = Number(exponent) * LOG2;
        Number E   = exp.floor();
        res        = res * (exp.fraction() * LN10).exp();

        char *s     = (res / TEN).round().to_str();
        int len     = strlen(s);
        s           = (char *)realloc(s, len + 10);
        char *exp_s = (E + ONE).to_str();
        s[len]      = 'e';
        strcpy(s + len + 1, exp_s);
        free(exp_s);
        return sign(s, Q.se);
    }
}
