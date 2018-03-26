#include "QFloat.h"

struct EFloat {
	uint8_t val[NUMBER_SIGNIFICAND_BYTES + 1]; //extended
	int16_t exp;
	bool sign;
	bool NaN;
	bool inf;
};

void shift_left(EFloat &f) {
	uint8_t out = 0;
	for (int i = 0; i <= NUMBER_SIGNIFICAND_BYTES; i++) {
		uint8_t n_out = f.val[i] >> 7;
		f.val[i] = (f.val[i] << 1) | out;
		out = n_out;
	}
}

void shift_right(EFloat &f) {
	uint8_t out = 0;
	for (int i = NUMBER_SIGNIFICAND_BYTES; i >= 0; i--) {
		uint8_t n_out = f.val[i] & 1;
		f.val[i] = (f.val[i] >> 1) | (out << 7);
		out = n_out;
	}
}

void neg(EFloat &a) {
	uint16_t carry = 1;
	for (int i = 0; i <= NUMBER_SIGNIFICAND_BYTES; i++) {
		uint16_t tmp = (uint16_t)(uint8_t)(~a.val[i]) + carry;
		carry = tmp >> 8;
		a.val[i] = tmp & ((1u << 8) - 1);
	}
	//assert(carry == 0)
}

EFloat to_efloat(const QFloat &a) {
	EFloat e;

	bool zero = true;
	for (int i = 0 ; i < NUMBER_SIGNIFICAND_BYTES; i++) {
		e.val[i] = a.val[i];
		zero = zero && (a.val[i] == 0);
	}
	
	e.exp = a.se & K;
	if (e.exp == K) { //special
		e.inf = zero;
		e.NaN = !zero;
	} else if(e.exp != 0) {
		e.inf = e.NaN = 0;
		e.val[NUMBER_SIGNIFICAND_BYTES] = 1; //not denomalized number
	} else {
		e.inf = e.NaN = 0;
		e.exp = 1; //denormalized number
	}

	e.sign = ((uint16_t)a.se) >> 15;

	return e;
}

EFloat add(EFloat a, EFloat b) {
	if (a.NaN)
		return a;
	if (b.NaN)
		return b;

	int diff_sign = a.sign ^ b.sign; 

	//check for Inf
	if (a.inf && b.inf) {
		a.NaN = diff_sign;
		a.inf = !diff_sign;
		return a;
	} else if (a.inf) {
		return a;
	} else if (b.inf) {
		return b;
	}

	//check exponential
	if (a.exp < b.exp) {
		EFloat tmp = a;
		a = b;
		b = tmp;
	}

	//normalize exp
	
	while (a.exp > b.exp) {
		shift_right(b);
		b.exp++;
	}

	//check size
	
	if (a.sign) neg(a);
	if (b.sign) neg(b);
	
	bool carry = 0;
	EFloat res;

	for (int i = 0; i <= NUMBER_SIGNIFICAND_BYTES; i++) {
		uint16_t tmp = (uint16_t)a.val[i] + b.val[i] + carry;
		carry = tmp >> 8;
		res.val[i] = (uint8_t)tmp;
	}
	res.exp = a.exp;
	res.sign = res.inf = res.NaN = 0;

	return res;
}

QFloat to_qfloat(EFloat e) {
	QFloat res;

	if (e.NaN)
		e.val[0] |= 1; // make it non-zero

	if (e.NaN || e.inf) {
		memcpy(res.val, e.val, sizeof(res.val));
		res.se = (((uint8_t)e.sign) << 7) | K;
		return res;
	}

	//check zero
	bool zero = true;

	for (int i = 0; i <= NUMBER_SIGNIFICAND_BYTES; i++)
		zero = zero && (e.val[i] == 0);

	if (zero) {
		memset(&res, 0, sizeof(res));
		return res;
	}
	//renormalize	
	bool sign = e.val[NUMBER_SIGNIFICAND_BYTES] >> 7;

	if (sign)
		neg(e);

	while (e.val[NUMBER_SIGNIFICAND_BYTES] > 1 || e.exp < 1) {
		shift_right(e);
		e.exp++;
	}

	while (e.val[NUMBER_SIGNIFICAND_BYTES] < 1 && e.exp > 1) {
		shift_left(e);
		e.exp--;
	}

	if (e.val[NUMBER_SIGNIFICAND_BYTES] < 1) //denomalized
		e.exp = 0;
	if (e.exp >= K) //infinite
		memset(e.val, 0, sizeof(e.val));

	res.se = (((uint16_t)sign) << 15) | e.exp;
	memcpy(res.val, e.val, sizeof(res.val));

	return res;
}

QFloat operator +(const QFloat &a, const QFloat &b) {
	EFloat x = to_efloat(a);
	EFloat y = to_efloat(b);

	return to_qfloat(add(x,y));
}

QFloat operator -(const QFloat &a) {
	QFloat res = a;
	res.se ^= (1u << 8);
	return res;
}

QFloat operator -(const QFloat &a, const QFloat &b) {
	return a + (-b);
}
