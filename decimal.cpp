#include <stdlib.h>
#include "QFloat.h"
#include "number/number.h"

const Number TWO(2);
const Number ONE(1);
const Number HALF("0.5");
const int16_t BIAS = 0b0011111111111111;

struct ParsedStr {
    bool negative          = false;
    const char *int_part   = NULL;
    int int_part_len       = 0;
    const char *float_part = NULL;
    int float_part_len     = 0;
    int exponent           = 0;
};

static ParsedStr parse_str(const char *s) {
#define IS_NUMERIC(C) ('0' <= (C) && (C) <= '9')
    ParsedStr p;
    const int len = strlen(s);

    if (len == 0)
        return p;

    bool has_dot = false, has_e = false;
    int dot_pos = len, e_pos = len;

    // Sanitize, find the position of dot and e
    for (int i = 0; i < len; i++) {
        if (s[i] == '-') {
            if (i != 0 && (s[i - 1] != 'E' && s[i - 1] != 'e'))
                return p;
        } else if (s[i] == '.') {
            if (has_dot || has_e)
                return p;
            has_dot = true;
            dot_pos = i;
        } else if (s[i] == 'E' || s[i] == 'e') {
            if (has_e)
                return p;
            if (i == 0 || !IS_NUMERIC(s[i - 1]))
                return p;
            has_e = true;
            e_pos = i;
        } else if (!IS_NUMERIC(s[i])) {
            return p;
        }
    }

    p.negative = s[0] == '-';

    // Find the start and len of int_part
    {
        int i = p.negative ? 1 : 0;
        if (i >= len || !IS_NUMERIC(s[i]))
            return p;
        while (s[i] == '0' && i + 1 < len && IS_NUMERIC(s[i + 1])) {
            i++;
        }
        p.int_part     = s + i;
        p.int_part_len = 0;
        for (int j = i; j < len && IS_NUMERIC(s[j]); j++) {
            p.int_part_len++;
        }
    }

    // Find the start and len of float_part
    if (has_dot) {
        int i = dot_pos + 1;
        if (i >= len || !IS_NUMERIC(s[i]))
            return p;
        p.float_part     = s + i;
        p.float_part_len = 0;
        for (int j = i; j < len && IS_NUMERIC(s[j]); j++) {
            p.float_part_len++;
        }
    } else {
        p.float_part     = NULL;
        p.float_part_len = 0;
    }

    // Parse the exponent
    if (has_e) {
        int i = e_pos + 1;
        if (i >= len)
            return p;
        bool negative = s[i] == '-';
        if (negative)
            i++;
        p.exponent = 0;
        for (; i < len; i++) {
            if (IS_NUMERIC(s[i])) {
                p.exponent *= 10;
                p.exponent += s[i] - '0';
            } else {
                return p;
            }
        }
        if (negative)
            p.exponent = -p.exponent;
    } else {
        p.exponent = 0;
    }

    return p;
}

// Ignoring exponent in s
QFloat Dec2QFloat(const char *s) {
    ParsedStr inp = parse_str(s);

    QFloat res;

    char *tmp = (char *)calloc(inp.int_part_len + 1, 1);
    memcpy(tmp, inp.int_part, inp.int_part_len);
    Number int_part(tmp);
    free(tmp);

    const int MAX_SIZE = sizeof(QFloat::val) * 8;  // 112
    uint8_t *bits      = (uint8_t *)calloc(MAX_SIZE, 1);

    int int_size = 0;
    while (int_part.sign != 0) {
        if (int_size < MAX_SIZE)
            bits[int_size] = int_part.d[PRECISION_SIZE] & 1;
        int_part = (int_part / TWO).floor();
        int_size++;
    }

    if (int_size > 0) {
        memmove(bits + (MAX_SIZE - int_size + 1), bits, int_size - 1);
        memset(bits, 0, MAX_SIZE - int_size + 1);
    }
    if (int_size < MAX_SIZE && inp.float_part_len > 0) {
        char *tmp = (char *)calloc(inp.float_part_len + 3, 1);
        strcpy(tmp, "0.");
        memcpy(tmp + 2, inp.float_part, inp.float_part_len);
        Number float_part(tmp);
        free(tmp);
        int i = MAX_SIZE - int_size;
        if (int_size == 0)
            i = MAX_SIZE - 1;
        for (; i >= 0; i--) {
            float_part = float_part * TWO;
            if (float_part >= ONE) {
                float_part = float_part.fraction();
                bits[i]    = 1;
            } else {
                bits[i] = 0;
            }
        }
    }

    int exponent = 0;
    bool is_zero = false;
    if (int_size > 0) {
        exponent = int_size - 1;
    } else {
        int i = MAX_SIZE - 1;
        while (i >= 0 && bits[i] == 0)
            i--;
        if (i < 0) {
            is_zero = true;
        } else {
            exponent = -(MAX_SIZE - i);
            memmove(bits + MAX_SIZE - i + 1, bits, i - 1);
            memset(bits, 0, MAX_SIZE - i + 1);
        }
    }

    for (int i = 0; i < MAX_SIZE; i++) {
        // Assumming the res.val is zero already
        if (bits[i] == 1) {
            res.val[i >> 3] |= 1 << (i & 7);
        }
    }

    free(bits);

    if (is_zero) {
        res.se &= 0b1000000000000000;
    } else {
        res.se &= 0b1000000000000000;
        res.se |= (BIAS + exponent);
    }

    if (inp.negative)
        res.se |= 1 << 15;

    return res;
}

char *QFloat2Dec(const QFloat &q) {
    int exponent = q.se & 0b0111111111111111;
    if (exponent == 0) {
        // denormalized or 0
        return Number("0").to_str();
    }

    exponent -= BIAS;
    const int BIT_COUNT = sizeof(QFloat::val) * 8;
    Number res(0ll);
    Number pow = TWO ^ Number(-BIT_COUNT + exponent);
    for (int i = 0; i < BIT_COUNT; i++) {
        if ((q.val[i >> 3] >> (i & 7)) & 1) {
            res = res + pow;
        }
        pow = pow * TWO;
    }
    res = res + pow;

    if (q.se < 0) res = -res;

    return res.to_str();
}
