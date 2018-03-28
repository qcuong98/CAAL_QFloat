#include "QFloat.h"

QFloat::QFloat() {
    se = 0;
    memset(val, 0, sizeof(val));
}

static QFloat inf() {
    QFloat f;
    f.se |= 0b0111111111111111;
    return f;
}

static QFloat nan() {
    QFloat f;
    f.se |= 0b0111111111111111;
    f.val[0] = 1;
    return f;
}

const QFloat QFloat::Inf = inf();
const QFloat QFloat::NaN = nan();

bool IsInf(const QFloat &q) {
    if ((q.se & 0b0111111111111111) != 0b0111111111111111) return false;
    for (int i=0; i<NUMBER_SIGNIFICAND_BYTES; i++) if (q.val[i] != 0) return false;
    return true;
}

bool IsNaN(const QFloat &q) {
    if ((q.se & 0b0111111111111111) != 0b0111111111111111) return false;
    for (int i=0; i<NUMBER_SIGNIFICAND_BYTES; i++) if (q.val[i] != 0) return true;
    return false;
}

bool IsZero(const QFloat &q) {
    if ((q.se & 0b0111111111111111) != 0) return false;
    for (int i=0; i<NUMBER_SIGNIFICAND_BYTES; i++) if (q.val[i] != 0) return false;
    return true;
}

