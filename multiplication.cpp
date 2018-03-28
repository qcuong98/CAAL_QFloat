#include "QFloat.h"

#define BIAS ((1u << (NUMBER_EXPONENT_BITS - 1)) - 1)

#define combine(sign, exp) ((((uint16_t)sign) << NUMBER_EXPONENT_BITS) | exp)

QFloat operator * (const QFloat& a, const QFloat& b) {
	/* handle denormalized floating point */

	uint16_t sign_a = (a.se >> NUMBER_EXPONENT_BITS);
	uint16_t exponent_a = a.se & K;
	uint16_t sign_b = (b.se >> NUMBER_EXPONENT_BITS);
	uint16_t exponent_b = b.se & K;

	uint16_t sign_c = sign_a ^ sign_b;
	

	QFloat c;
	/* (1+x) * (1+y) = 1 + x + y + x*y */


	if (IsInf(a) || IsInf(b)) { //inf
		c.val[0] |= IsZero(a) || IsZero(b); //inf * zero is Na	
		c.se = combine(sign_c, K);
		return c;
	}

	if (IsZero(a) || IsNaN(a))
		return a;
	if (IsZero(b) || IsNaN(b))
		return b;

	/* not process denormalized number */

	bool denom_a = false, denom_b = false;
	if (exponent_a == 0) {
		denom_a = true;
		exponent_a = 1;
	}

	if (exponent_b == 0) {
		denom_b = true;
		exponent_b = 1;
	}

	int32_t exponent_c = (int32_t)exponent_a + exponent_b - BIAS;
	uint16_t tmp;

	uint8_t c_val[NUMBER_SIGNIFICAND_BYTES * 2 + 1];
	memset(c_val, 0, sizeof(c_val));

	/* x*y */
	for (int i = 0; i < NUMBER_SIGNIFICAND_BYTES; ++i)
		for (int j = 0; j < NUMBER_SIGNIFICAND_BYTES; ++j) {
			int k = i + j;

			tmp = (uint16_t)a.val[i] * b.val[j];
			for (;tmp && k <= NUMBER_SIGNIFICAND_BYTES * 2; ++k) {
				tmp += c_val[k];
				c_val[k] = tmp & UINT8_MAX;
				tmp >>= 8;
			}
		}

	/* +x if y is not denom */
	if (denom_a && denom_b)
		goto result;

	tmp = 0;

	for (int i = 0; i < NUMBER_SIGNIFICAND_BYTES; ++i) {
		
		if (!denom_b)
			tmp += a.val[i];
		if (!denom_a)
			tmp += b.val[i];

		tmp += c_val[i + NUMBER_SIGNIFICAND_BYTES];
		c_val[i + NUMBER_SIGNIFICAND_BYTES] = tmp & UINT8_MAX;
		tmp >>= 8;
	}
	c_val[NUMBER_SIGNIFICAND_BYTES * 2] += tmp; 

	/* + 1 */
	if (!denom_a && !denom_b)
		c_val[NUMBER_SIGNIFICAND_BYTES * 2] += 1;

result:
	while (c_val[NUMBER_SIGNIFICAND_BYTES * 2] > 1 || exponent_c < 1) {
		/* significand shift right 1 */
		bool zero = 1;
		uint8_t l_bit = 0, r_bit;
		for (int k = NUMBER_SIGNIFICAND_BYTES * 2; k >= NUMBER_SIGNIFICAND_BYTES; --k) {
			r_bit = (c_val[k] & 1);
			c_val[k] = (c_val[k] >> 1) | (l_bit << 7);
			zero = zero && (c_val[k] == 0);
			l_bit = r_bit;
		}

		if (zero)
			exponent_c = 1;
		else
			++exponent_c;
	}

	if (exponent_c >= K) {  //overflow
		c.se = (sign_c << NUMBER_EXPONENT_BITS) | K;
		memset(c.val, 0, sizeof(c.val));
		return c; //return INF
	}
	
	c.se = sign_c << NUMBER_EXPONENT_BITS;

	if (c_val[NUMBER_SIGNIFICAND_BYTES * 2]) //not denormalized
		c.se |= exponent_c;

	memcpy(c.val, c_val + NUMBER_SIGNIFICAND_BYTES, sizeof(c.val));
	return c;
}

bool ge(uint8_t *a, uint8_t *b) {
	for (int k = NUMBER_SIGNIFICAND_BYTES * 2; k >= 0; --k) {
		if (a[k]!=b[k])
			return a[k] > b[k];
	}
	return 1;
}

void shift_right(uint8_t *a) {
	uint8_t l_bit = 0, r_bit;
	for (int k = NUMBER_SIGNIFICAND_BYTES * 2; k >= 0; --k) {
		r_bit = (a[k] & 1);
		a[k] = (a[k] >> 1) | (l_bit << 7);
		l_bit = r_bit;
	}
}

void shift_left(uint8_t *a) {
	uint8_t l_bit, r_bit = 0;
	for (int k = 0; k <= NUMBER_SIGNIFICAND_BYTES * 2; ++k) {
		l_bit = (a[k] >> 7);
		a[k] = (a[k] << 1) | r_bit;
		r_bit = l_bit;
	}
}

QFloat operator /(const QFloat &a, const QFloat &b) {
	uint16_t sign_a = (a.se >> NUMBER_EXPONENT_BITS);
	int32_t exponent_a = a.se & K;
	uint16_t sign_b = (b.se >> NUMBER_EXPONENT_BITS);
	int32_t exponent_b = b.se & K;

	uint16_t sign_c = sign_a ^ sign_b;
	

	QFloat c;

	if (IsNaN(a))
		return a; //NaN
	if (IsNaN(b))
		return b;

	if (IsInf(b)) { //b is +inf
		if (exponent_a == K) //NaN or inf 
			return QFloat::NaN; //NaN

		c.se = combine(sign_c, 0);
		return c; //signed zero
	}
	
	if (IsZero(b)) { // b is zero
		if (IsZero(a)) //a is zero
			return QFloat::NaN;

		c.se = combine(sign_c, K);
		return c;
	}

	if (IsZero(a)) { //a is zero
		c.se = combine(sign_c, 0);
		return c;
	}


	uint8_t x_val[NUMBER_SIGNIFICAND_BYTES * 2 + 1];
	memset(x_val, 0, sizeof(x_val));
	memcpy(x_val + NUMBER_SIGNIFICAND_BYTES, a.val, sizeof(a.val));

	uint8_t y_val[NUMBER_SIGNIFICAND_BYTES * 2 + 1];
	memset(y_val, 0, sizeof(y_val));
	memcpy(y_val + NUMBER_SIGNIFICAND_BYTES, b.val, sizeof(b.val));


	if (exponent_a != 0) { //denom 
		x_val[NUMBER_SIGNIFICAND_BYTES * 2] = 1;
	} else {
		exponent_a = 1;
		while (x_val[NUMBER_SIGNIFICAND_BYTES * 2] == 0) {
			--exponent_a;
			shift_left(x_val);
		}
	}

	if (exponent_b != 0) { //denom 
		y_val[NUMBER_SIGNIFICAND_BYTES * 2] = 1;
	} else {
		exponent_b = 1;
		while (y_val[NUMBER_SIGNIFICAND_BYTES * 2] == 0) {
			--exponent_b;
			shift_left(y_val);
		}
	}

	if (!ge(x_val, y_val)) {
		shift_left(x_val);
		--exponent_a;
	}

	int32_t exponent_c = exponent_a - exponent_b + BIAS;

	if (exponent_c >= K) { //infinite
		c.se = combine(sign_c, K);
		return c;
	}

	uint8_t c_val[2 * NUMBER_SIGNIFICAND_BYTES + 1];
	memset(c_val, 0, sizeof(c_val));

	int i1 = NUMBER_SIGNIFICAND_BYTES;
	int i2 = 0;
	while (i1 >= 0) {
		if (ge(x_val, y_val)) { //x>=y	
			// minus x -= y		
			int16_t carry = 0;
			for (int j = 0; j <= 2*NUMBER_SIGNIFICAND_BYTES; j++) {
				int16_t tmp = (int16_t)x_val[j] - y_val[j] - carry;
				if (tmp < 0){
					carry = 1;
					tmp += (1u << 8);
				} else {
					carry = 0;
				}
				x_val[j] = tmp;
			}
			c_val[i1] |= 1u << i2;
		}

		shift_right(y_val);

		if (i2) {
			--i2;
		} else {
			--i1;
			i2 = 7;
		}
	}

	while (exponent_c <= 0) {
		//denom number
		shift_right(c_val);
		++exponent_c;
	}

	if (c_val[NUMBER_SIGNIFICAND_BYTES] == 0)
		exponent_c = 0;

	c.se = combine(sign_c, exponent_c);
	memcpy(c.val, c_val, sizeof(c.val));
	return c;	
}
