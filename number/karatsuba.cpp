#include "karatsuba.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

#define LL long long
#define min(x, y) ((x) < (y) ? (x) : (y))


namespace Karatsuba {
// Must be the same as in number.cpp
const LL B    = 100000000;
const LL SIZE = 8;
const LL M[]  = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000};

void print(num x) {
    if (x.n == 0)
        printf("0");
    else {
        printf("%lld", x.d[x.n - 1]);
        for (int i = x.n - 1; i--;)
            printf("%08lld", x.d[i]);
    }
    printf("\n");
}

num alloc(int size) {
    num x;
    x.n = size;
    x.d = (LL *)calloc(size, sizeof(LL));
    return x;
}

num zero() {
    num x;
    x.n = 0;
    x.d = nullptr;
    return x;
}

// remove leading zeroes
void shift(num *r) {
    LL *i = r->d + (r->n - 1);
    while (r->n > 0 && *i == 0)
        r->n--, i--;
}

// x += a
void sum(LL *x, num a) {
    LL rem = 0;
    for (LL *i = a.d, *e = a.d + a.n; i != e; i++, x++) {
        rem += *i + *x;
        if (rem >= B) {
            *x  = rem - B;
            rem = 1;
        } else {
            *x  = rem;
            rem = 0;
        }
    }
    for (; rem > 0; x++) {
        rem += *x;
        if (rem >= B) {
            *x  = rem - B;
            rem = 1;
        } else {
            *x  = rem;
            rem = 0;
        }
    }
}

// x -= a
void sub(LL *x, num a) {
    LL rem = 0;
    for (LL *i = a.d, *e = a.d + a.n; i != e; i++, x++) {
        rem += *x - *i;
        if (rem < 0) {
            *x  = rem + B;
            rem = -1;
        } else {
            *x  = rem;
            rem = 0;
        }
    }
    for (; rem < 0; x++) {
        rem += *x;
        if (rem < 0) {
            *x  = rem + B;
            rem = -1;
        } else {
            *x  = rem;
            rem = 0;
        }
    }
}

// a += b * x
void mul(LL *a, num b, LL x) {
    LL rem = 0;
    for (LL *i = b.d, *e = b.d + b.n; i != e; i++, a++) {
        rem += *a + *i * x;
        *a = rem % B;
        rem /= B;
    }
    for (; rem > 0; a++) {
        rem += *a;
        *a++ = rem % B;
        rem /= B;
    }
}

// compute size to split
int compute(int x, int y) {
    int r = min(x, y) / 2;
    if (x - r == r || y - r == r)
        return r - 1;
    return r;
}

void split(num x, num *x0, num *x1, int m) {
    x0->n = m;
    x0->d = x.d;
    x1->n = x.n - m;
    x1->d = x.d + m;
    shift(x0);
    shift(x1);
}

num copy(num x) {
    num r = alloc(x.n);
    memcpy(r.d, x.d, x.n * sizeof(LL));
    return r;
}

// xy = (B^2m + b^m)x1y1 + (b + 1)x0y0 - b(x1 - x0)(y1 - y0)
// With:
// x1*B^m + x0 = x
// y1*B^m + y0 = y
// x1 > x0
// y1 > y0
num karatsuba(num x, num y) {
    if (x.n == 0 || y.n == 0)
        return zero();
    if (x.n <= 16) {
        num t = x;
        x     = y;
        y     = t;
    }
    if (y.n <= 16) {
        // perform grade-school multiplication
        num r = alloc(x.n + y.n);
        for (int i = 0; i < y.n; i++) {
            mul(r.d + i, x, y.d[i]);
        }
        shift(&r);
        return r;
    }
    num x0, x1, y0, y1;
    int m = compute(x.n, y.n);
    split(x, &x0, &x1, m);
    split(y, &y0, &y1, m);
    num xy0 = karatsuba(x0, y0);
    num xy1 = karatsuba(x1, y1);
    num r   = alloc(x.n + y.n);
    if (xy0.n > 0)
        memcpy(r.d, xy0.d, xy0.n * sizeof(LL));
    sum(r.d + m, xy0);
    sum(r.d + m, xy1);
    sum(r.d + 2 * m, xy1);
    free(xy0.d);
    free(xy1.d);
    num sx = copy(x1);
    sub(sx.d, x0);
    shift(&sx);
    num sy = copy(y1);
    sub(sy.d, y0);
    shift(&sy);
    num xy = karatsuba(sx, sy);
    sub(r.d + m, xy);
    shift(&r);
    free(xy.d);
    free(sx.d);
    free(sy.d);
    return r;
}
};  // namespace Karatsuba
