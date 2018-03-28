#include <stdlib.h>
#include "QFloat.h"
#include "number/number.h"

const int16_t BIAS     = 0b0011111111111111;
const int MAX_EXPONENT = 0b0111111111111111 - BIAS;
const int MIN_EXPONENT = -BIAS;
const QFloat TEN32     = Bin2QFloat(
    "0_100000001101001_"
    "0011101110001011010110110101000001010110111000010110101100111011111000000100000000000000000000"
    "000000000000000000");
const QFloat TEN = Bin2QFloat(
    "0_100000000000010_"
    "0100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
    "000000000000000000");

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

QFloat Dec2QFloat(const char *s) {
#define GET_I(_x) (_x.sign == 0 ? 0 : _x.d[PRECISION_SIZE])
    if (0 == strcmp(s, "+Inf"))
        return QFloat::Inf;
    if (0 == strcmp(s, "-Inf"))
        return -QFloat::Inf;
    if (0 == strcmp(s, "NaN"))
        return QFloat::NaN;

    ParsedStr inp = parse_str(s);

    Number int_part, float_part;
    {
        ParsedStr inp = parse_str(s);
        char *tmp     = (char *)calloc(inp.int_part_len + 1, 1);
        memcpy(tmp, inp.int_part, inp.int_part_len);
        Number _int_part(tmp);
        free(tmp);

        Number _float_part(0ll);
        if (inp.float_part_len > 0) {
            tmp = (char *)calloc(inp.float_part_len + 3, 1);
            strcpy(tmp, "0.");
            memcpy(tmp + 2, inp.float_part, inp.float_part_len);
            _float_part = tmp;
            free(tmp);
        }

        int_part   = _int_part;
        float_part = _float_part;
    }

    const int MAX_SIZE = sizeof(QFloat::val) * 8;  // 112

#define FREE              \
    {                     \
        free(int_bits);   \
        free(float_bits); \
        free(bits);       \
    }
    uint8_t *bits       = (uint8_t *)calloc(MAX_SIZE, 1);
    uint8_t *int_bits   = (uint8_t *)calloc(MAX_SIZE, 1);
    uint8_t *float_bits = (uint8_t *)calloc(MAX_SIZE, 1);

    int int_size = 0;
    while (int_part.sign != 0) {
        int_bits[int_size % MAX_SIZE] = GET_I(int_part) & 1;
        int_part                      = (int_part / TWO).floor();
        int_size++;
    }

    for (int i = MAX_SIZE - 1; i >= 0; i--) {
        float_part    = float_part * TWO;
        float_bits[i] = GET_I(float_part.floor()) != 0;
        if (float_bits[i])
            float_part = float_part.fraction();
    }

    int exponent = int_size - 1;
    if (int_size == 0) {
        exponent = -1;
        for (int i = MAX_SIZE - 1; i >= 0 && float_bits[i] == 0; i--) {
            exponent--;
        }
        if (exponent < -MAX_SIZE) {
            FREE;
            return QFloat();
        }
    }

    if (exponent >= MAX_EXPONENT) {
        FREE;
        return inp.negative ? -QFloat::Inf : QFloat::Inf;
    } else if (exponent <= MIN_EXPONENT) {
        int shift = MIN_EXPONENT - exponent;
        if (shift >= MAX_SIZE) {
            FREE;
            return QFloat();
        }
        for (int i = MAX_SIZE - shift - 1, float_i = MAX_SIZE - 1; i >= 0; i--) {
            if (int_size > 0) {
                int_size--;
                bits[i] = int_bits[int_size % MAX_SIZE];
            } else {
                bits[i] = float_bits[float_i];
                float_i--;
            }
        }
        exponent = 0;
    } else {
        int float_i = MAX_SIZE - 1;
        if (int_size == 0) {
            while (float_i >= 0 && float_bits[float_i] == 0)
                float_i--;
            float_i--;
        }
        int_size--;
        for (int i = MAX_SIZE - 1; i >= 0; i--) {
            if (int_size > 0) {
                int_size--;
                bits[i] = int_bits[int_size % MAX_SIZE];
            } else if (float_i >= 0) {
                bits[i] = float_bits[float_i];
                float_i--;
            }
        }
        exponent += BIAS;
    }

    QFloat res;
    if (inp.negative)
        res.se |= 0b1000000000000000;
    res.se |= exponent;
    for (int i = 0; i < MAX_SIZE; i++) {
        if (bits[i] == 1)
            res.val[i >> 3] |= 1 << (i & 7);
    }
    FREE;

    if (inp.exponent != 0) {
        bool neg = inp.exponent < 0;
        int exp  = neg ? -inp.exponent : inp.exponent;
        for (int c = exp >> 5; c > 0; c--) {
            res = neg ? res / TEN32 : res * TEN32;
        }
        for (int c = exp & 31; c > 0; c--) {
            res = neg ? res / TEN : res * TEN;
        }
    }

    return res;
}
