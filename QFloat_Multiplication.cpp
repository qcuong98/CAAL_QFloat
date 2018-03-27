#include "QFloat.h"

#define BIAS ((1u << (NUMBER_EXPONENT_BITS - 1)) - 1)
#define same(a, b) (memcmp(&(a), &(b), sizeof(a)) == 0)

QFloat operator * (const QFloat& a, const QFloat& b) {
	/* handle denormalized floating point */

	uint16_t sign_a = (a.se >> NUMBER_EXPONENT_BITS);
	uint16_t exponent_a = a.se & K;
	uint16_t sign_b = (b.se >> NUMBER_EXPONENT_BITS);
	uint16_t exponent_b = b.se & K;

	uint16_t sign_c = sign_a ^ sign_b;
	
	int32_t exponent_c = (int32_t)exponent_a + exponent_b - BIAS;

	QFloat c;
	/* (1+x) * (1+y) = 1 + x + y + x*y */

	if ((exponent_a == K && same(a.val, c.val)) || (exponent_b == K && same(b.val, c.val))) { //inf
		c.se = (sign_c << NUMBER_EXPONENT_BITS) | K;
		c.val[0] |= same(a, c) || same(b, c); //inf * zero is NaN
		return c;
	}

	if (exponent_a == K && !same(a.val, c.val))
		return a; //NaN
	if (exponent_b == K && !same(b.val, c.val))
		return b;

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

	/* x + y */
	tmp = 0;
	for (int i = 0; i < NUMBER_SIGNIFICAND_BYTES; ++i) {
		tmp += (int16_t)a.val[i] + b.val[i] + c_val[i + NUMBER_SIGNIFICAND_BYTES];
		c_val[i + NUMBER_SIGNIFICAND_BYTES] = tmp & UINT8_MAX;
		tmp >>= 8;
	}
	c_val[NUMBER_SIGNIFICAND_BYTES * 2] += tmp; 

	/* + 1 */
	c_val[NUMBER_SIGNIFICAND_BYTES * 2] += 1;

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

#define ge(a, b) (memcmp((a),(b),sizeof(a)) >= 0)

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
	uint16_t exponent_a = a.se & K;
	uint16_t sign_b = (b.se >> NUMBER_EXPONENT_BITS);
	uint16_t exponent_b = b.se & K;

	uint16_t sign_c = sign_a ^ sign_b;
	
	int32_t exponent_c = (int32_t)exponent_a - exponent_b + BIAS;

	uint8_t x_val[NUMBER_SIGNIFICAND_BYTES * 2 + 1];
	memset(x_val, 0, sizeof(x_val));
	memcpy(x_val + NUMBER_SIGNIFICAND_BYTES, a.val, sizeof(a.val));

	uint8_t y_val[NUMBER_SIGNIFICAND_BYTES * 2 + 1];
	memset(y_val, 0, sizeof(y_val));
	memcpy(y_val + NUMBER_SIGNIFICAND_BYTES, b.val, sizeof(b.val));

	x_val[NUMBER_SIGNIFICAND_BYTES * 2] = 1;
	y_val[NUMBER_SIGNIFICAND_BYTES * 2] = 1;

	if (!ge(x_val, y_val)) {
		shift_left(y_val);
		--exponent_c;
	}

	uint8_t c_val[NUMBER_SIGNIFICAND_BYTES + 1];
	memset(c_val, 0, sizeof(c_val));

	int i1 = NUMBER_SIGNIFICAND_BYTES;
	int i2 = 0;
	while (i1 >= 0) {
		if (ge(x_val, y_val)) { //x>=y	
			// minus x -= y		
			int carry = 0;
			for (int j = 0; j <= 2*NUMBER_SIGNIFICAND_BYTES; j++) {
				int16_t tmp = (int16_t)x_val[j] - y_val[j] - carry;
				if (tmp < 0){
					carry = 1;
					tmp += UINT8_MAX;
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

	QFloat c;
	c.se = (sign_c << NUMBER_EXPONENT_BITS) | exponent_c;
	memcpy(c.val, c_val, sizeof(c.val));
	return c;	
}
