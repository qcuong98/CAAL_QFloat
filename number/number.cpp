#include "number.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include "exception.h"
#include "karatsuba.h"

namespace number {
bool is_number(char c) {
    return '0' <= c && c <= '9';
}
};  // namespace number

const Number one(1ll);
const Number two(2ll);
const Number ln10("2.30258509299404568401799145468436420760110148862877297603332790096757260967735248023599720508959829834");
const Number PI("3.141592653589793238462643383279502884197169399375105820974944592307816406286");

using namespace number;

Number::~Number() {
    free(d);
}

Number::Number() : sign(0), n(0), d(nullptr) {}

Number::Number(int sign, int n, LL *d) : sign(sign), n(n), d(d) {
    unpad();
}

Number::Number(const Number &x) : sign(x.sign), n(x.n), d(nullptr) {
    if (n > 0) {
        d = (LL *)malloc(n * sizeof(LL));
        if (d == nullptr)
            throw Exception("Fatal: Allocation error");
        memcpy(d, x.d, n * sizeof(LL));
    }
}

Number::Number(LL x) : sign(1), n(4 + PRECISION_SIZE) {
    d = (LL *)calloc(n, sizeof(LL));
    if (d == nullptr)
        throw Exception("Fatal: Allocation error");
    if (x < 0) {
        sign = -1;
        x    = -x;
    }
    LL *p = d + PRECISION_SIZE;
    while (x > 0) {
        *p = x % B;
        p++;
        x /= B;
    }
    unpad();
}

Number &Number::operator=(const Number &x) {
    sign = x.sign;
    n    = x.n;
    free(d);
    d = nullptr;
    if (n > 0) {
        d = (LL *)malloc(n * sizeof(LL));
        if (d == nullptr)
            throw Exception("Fatal: Allocation error");
        memcpy(d, x.d, n * sizeof(LL));
    }
    return *this;
}

Number::Number(const char *s) : Number() {
    if (*s == 0)
        return;

    int count       = 0;
    int count_frac  = INT_MIN;
    const char *pos = nullptr;
    for (pos = s; *pos; pos++) {
        if (is_number(*pos))
            count++, count_frac++;
        else if (*pos == '.')
            count_frac = 0;
        if (count_frac == PRECISION)
            break;
    }
    if (count_frac < 0)
        count_frac = 0;

    count    = count - count_frac;
    int size = count / SIZE + (count % SIZE != 0) + PRECISION_SIZE;

    if (*s == '-')
        sign = -1;
    else
        sign = 1;

    n = size;
    d = (LL *)calloc(n, sizeof(LL));
    if (d == nullptr)
        throw Exception("Fatal: Allocation error");

    int add_zero = PRECISION - count_frac;
    for (int i = 0; i < n; i++) {
        // d[i] = 0;
        for (int c = 0; c < SIZE; c++) {
            if (add_zero > 0) {
                add_zero -= 1;
            } else {
                while (pos >= s && !is_number(*pos))
                    pos -= 1;
                if (pos >= s) {
                    d[i] += M[c] * (*pos - '0');
                    pos -= 1;
                }
            }
        }
    }

    unpad();
}

void Number::unpad() {
    if (sign == 0) {
        n = 0;
        free(d);
        d = nullptr;
        return;
    }
    while (n > 0 && d[n - 1] == 0)
        n--;
    if (n == 0) {
        free(d);
        d    = nullptr;
        sign = 0;
    } else {
        void *nd = realloc(d, n * sizeof(LL));
        if (nd == nullptr) {
            free(d);
            d = nullptr;
            throw Exception("Fatal: Allocation error");
        }
        d = (LL *)nd;
    }
}

std::ostream &operator<<(std::ostream &os, const Number &x) {
    if (x.sign == 0)
        os << '0';
    else {
        if (x.sign < 0)
            os << '-';
        if (x.n > PRECISION_SIZE)
            os << x.d[x.n - 1];
        else
            os << '0';
        for (int i = x.n - 2; i >= PRECISION_SIZE; i--) {
            os << std::setw(SIZE) << std::setfill('0') << x.d[i];
        }
        std::ostringstream buffer;
        for (int i = PRECISION_SIZE - 1; i >= 0; i--) {
            buffer << std::setw(SIZE) << std::setfill('0') << (i < x.n ? x.d[i] : 0);
        }
        std::string str(buffer.str());
        while (!str.empty() && str.back() == '0')
            str.pop_back();
        if (!str.empty()) {
            os << '.' << str;
        }
    }
    return os;
}

void Number::shift(int k) {
    if (sign == 0)
        return;
    if (k > 0) {
        d = (LL *)realloc(d, (n + k) * sizeof(LL));
        if (d == nullptr)
            throw Exception("Fatal: Allocation error");
        memmove(d + k, d, n * sizeof(LL));
        memset(d, 0, k * sizeof(LL));
        n += k;
    } else if (k < 0) {
        if (n + k <= 0) {
            sign = 0;
            n    = 0;
            free(d);
            d = nullptr;
            return;
        }
        memmove(d, d - k, (n + k) * sizeof(LL));
        n += k;
        d = (LL *)realloc(d, n * sizeof(LL));
        if (d == nullptr)
            throw Exception("Fatal: Allocation error");
    }
}

Number Number::operator-() const {
    Number x(*this);
    x.sign = -x.sign;
    return x;
}

Number Number::raw_add(const Number &a, const Number &b) {
    int m = std::max(a.n, b.n) + 1;
    int k = std::min(a.n, b.n);
    LL *d = (LL *)malloc(m * sizeof(LL));
    if (d == nullptr)
        throw Exception("Fatal: Allocation error");
    LL rem = 0;
#define UPDATE(x, rem)        \
    if (rem >= B)             \
        x = rem - B, rem = 1; \
    else                      \
        x = rem, rem = 0
    for (int i = 0; i < k; i++) {
        rem += a.d[i] + b.d[i];
        UPDATE(d[i], rem);
    }
    for (; k < a.n; k++) {
        rem += a.d[k];
        UPDATE(d[k], rem);
    }
    for (; k < b.n; k++) {
        rem += b.d[k];
        UPDATE(d[k], rem);
    }
#undef UPDATE
    d[m - 1] = rem;
    return Number(1, m, d);
}

int Number::raw_compare(const Number &a, const Number &b) {
    if (a.n < b.n)
        return -1;
    if (a.n > b.n)
        return 1;
    for (int i = a.n - 1; i >= 0; i--) {
        if (a.d[i] < b.d[i])
            return -1;
        if (a.d[i] > b.d[i])
            return 1;
    }
    return 0;
}
#define CMP(op, val)                                  \
    bool Number::operator op(const Number &x) const { \
        if (sign < x.sign)                            \
            return -1 val;                            \
        if (sign > x.sign)                            \
            return 1 val;                             \
        return sign * raw_compare(*this, x) val;      \
    }
CMP(<, == -1);
CMP(>, == 1);
CMP(<=, != 1);
CMP(>=, != -1);
CMP(==, == 0);
CMP(!=, != 0);

Number Number::raw_substract(const Number &a, const Number &b) {
    switch (raw_compare(a, b)) {
        case 0:
            return Number();
        case -1:
            Number x = raw_substract(b, a);
            x.sign   = -1;
            return x;
    }

    LL *d = (LL *)calloc(a.n, sizeof(LL));

    LL rem = 0;
#define UPDATE(x, rem)        \
    if (rem < 0)              \
        x = rem + B, rem = 1; \
    else                      \
        x = rem, rem = 0
    for (int i = 0; i < b.n; i++) {
        rem = a.d[i] - b.d[i] - rem;
        UPDATE(d[i], rem);
    }
    for (int i = b.n; i < a.n; i++) {
        rem = a.d[i] - rem;
        UPDATE(d[i], rem);
    }
#undef UPDATE
    return Number(1, a.n, d);
}

Number Number::operator+(const Number &x) const {
    if (sign == 0)
        return x;
    if (x.sign == 0)
        return *this;
    if (sign > 0 && x.sign > 0)
        return raw_add(*this, x);
    if (sign < 0 && x.sign < 0)
        return raw_add(*this, x).invert();
    if (sign > 0 && x.sign < 0)
        return raw_substract(*this, x);
    if (sign < 0 && x.sign > 0)
        return raw_substract(x, *this);
    throw Exception("Fatal: Unhandled exception in add");
}

Number Number::operator-(const Number &x) const {
    if (sign == 0)
        return Number(x).invert();
    if (x.sign == 0)
        return *this;
    if (sign > 0 && x.sign > 0)
        return raw_substract(*this, x);
    if (sign > 0 && x.sign < 0)
        return raw_add(*this, x);
    if (sign < 0 && x.sign > 0)
        return raw_add(*this, x).invert();
    if (sign < 0 && x.sign < 0)
        return raw_substract(x, *this);
    throw Exception("Fatal: Unhandled exception in subtract");
}

Number Number::raw_multiply(const Number &x, const Number &y) {
    if (x.sign == 0 || y.sign == 0)
        return Number();
    Karatsuba::num a  = {x.n, x.d};
    Karatsuba::num b  = {y.n, y.d};
    Karatsuba::num ab = Karatsuba::karatsuba(a, b);
    if (ab.d == nullptr)
        return Number();
    return Number(1, ab.n, ab.d);
}

Number Number::operator*(const Number &x) const {
    Number a = raw_multiply(*this, x);
    a.shift(-PRECISION_SIZE);
    a.sign = sign * x.sign;
    return a;
}

Number Number::raw_divide(const Number &A, const Number &b) {
    if (A.sign == 0)
        return Number();
    Number a(A);
    Number Q;
    while (raw_compare(a, b) != -1) {
        int k = 0;
        {
            Number B(b);
            k = a.n - b.n;
            B.shift(k);
            if (raw_compare(a, B) == -1)
                k--;
        }

        Number q(one);
        q.shift(-PRECISION_SIZE + k);

        LL l = 1, r = B - 1;
        while (l <= r) {
            LL v         = (l + r) / 2;
            q.d[q.n - 1] = v;
            Number A     = raw_multiply(b, q);
            if (raw_compare(A, a) != 1)
                l = v + 1;
            else
                r = v - 1;
        }

        assert(r != 0);
        q.d[q.n - 1] = r;
        Q            = Q + q;
        a            = raw_substract(a, raw_multiply(q, b));
    }
    return Q;
}

Number Number::operator/(const Number &x) const {
    if (x.sign == 0)
        throw Exception("NaN");
    Number y(*this);
    y.shift(PRECISION_SIZE);
    Number a = raw_divide(y, x);
    a.sign   = sign * x.sign;
    return a;
}

Number Number::intpow(const Number &a) const {
    if (a.n > PRECISION_SIZE + 1)
        throw Exception("Number is too big");
    if (a.n <= PRECISION_SIZE)
        return Number(one);
    LL p = a.d[PRECISION_SIZE];
    if (SIZE * (n - PRECISION_SIZE) * p > 5000000)
        throw Exception("Number is too big");
    Number res(one);
    Number tmp(*this);
    while (p > 0) {
        if (p & 1)
            res = res * tmp;
        p >>= 1;
        tmp = tmp * tmp;
    }
    return res;
}

Number &Number::invert() {
    sign = -sign;
    return *this;
}

Number Number::log() const {
    if (sign <= 0)
        throw Exception("NaN");

    Number a(*this);
    int k = a.n - PRECISION_SIZE;
    a.shift(-k);
    {
        LL msd = a.d[PRECISION_SIZE - 1];
        int c  = 0;
        while (msd < B)
            msd *= 10, c++;
        c--;
        Number t(one);
        for (int i = 0; i < c; i++)
            t.d[PRECISION_SIZE] *= 10;
        a = a * t;
        k = -(-k * 8 + c);
    }

    Number x = a - one;
    Number T(x);
    x = x * x;

    Number y = a + one;
    Number M(y);
    y = y * y;

    Number n(one);
    Number sum;
    for (int i = 0; i <= 50; i++) {
        sum = sum + T / M / n;
        n.d[PRECISION_SIZE] += 2;
        T = T * x;
        M = M * y;
    }

    sum = two * sum;
    Number K(one);
    K.d[PRECISION_SIZE] = k;
    return sum + K * ln10;
}

Number Number::fraction() const {
    if (sign == 0)
        return Number();
    Number f(one);
    memcpy(f.d, d, PRECISION_SIZE * sizeof(LL));
    f.d[PRECISION_SIZE] = 0;
    f.unpad();
    return f;
}

Number Number::operator^(const Number &b) const {
    if (b.sign < 0)
        return one / (*this ^ (-b));
    Number f(b.fraction());
    if (f.sign == 0)
        return intpow(b);
    Number x = f * log();
    Number p = intpow(b);
    Number m(one);
    Number n(one);
    Number res;
    for (LL i = 1; i < 50; i++) {
        Number tmp = p / m;
        if (tmp.sign == 0)
            break;
        res                 = res + tmp;
        n.d[PRECISION_SIZE] = i;
        m                   = m * n;
        p                   = p * x;
    }
    return res;
}

Number Number::floor() const {
    Number r(*this);
    if (r.sign != 0) {
        memset(r.d, 0, sizeof(LL) * std::min(PRECISION_SIZE, r.n));
        r.unpad();
    }
    return r;
}

Number Number::abs() const {
    Number r(*this);
    if (sign < 0)
        r.invert();
    return r;
}

Number Number::exp() const {
    Number T(one);
    Number M(one);
    Number r;
    Number N(one);
    for (int i = 1; i < 500; i++) {
        Number tmp = T / M;
        if (tmp.sign == 0)
            break;
        r                   = r + tmp;
        N.d[PRECISION_SIZE] = i;
        M                   = M * N;
        T                   = T * *this;
    }
    return r;
}

char *Number::to_str() const {
    std::stringstream ss;
    ss << *this;
    const std::string s = ss.str();
    char *res           = (char *)malloc(s.length() + 1);
    res[s.length()]     = '\0';
    memcpy(res, s.data(), s.length());
    return res;
}

Number Number::chop(int k) const {
    Number res(*this);
    if (res.sign != 0)
    for (int i=0; i<PRECISION_SIZE - k; i++) res.d[i] = 0;
    return res;
}
