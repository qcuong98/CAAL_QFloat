#include "QFloat.h"

#define BIAS ((1u << (NUMBER_EXPONENT_BITS - 1)) - 1)

QFloat operator * (const QFloat& a, const QFloat& b) {
	/* handle denormalized floating point */

	bool sign_a = (a.se >> NUMBER_EXPONENT_BITS);
	uint16_t exponent_a = a.se & K;
	bool sign_b = (b.se >> NUMBER_EXPONENT_BITS);
	uint16_t exponent_b = b.se & K;

	bool sign_c = sign_a ^ sign_b;
	uint16_t exponent_c = exponent_a + exponent_b - BIAS;

	/* (1+x) * (1+y) = 1 + x + y + x*y */

	uint16_t tmp;
	int8_t n = 1;

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


	while (c_val[NUMBER_SIGNIFICAND_BYTES * 2] > 1) {
		/* significand shift right 1 */
		uint8_t l_bit = 0, r_bit;
		for (int k = NUMBER_SIGNIFICAND_BYTES * 2; k >= 0; --k) {
			r_bit = (c_val[k] & 1);
			c_val[k] = (c_val[k] >> 1) | (l_bit << 7);
			l_bit = r_bit;
		}
		++exponent_c;
	}

	QFloat c;

	c.se = exponent_c | (((uint16_t)sign_c) << NUMBER_EXPONENT_BITS);
	memcpy(c.val, c_val + NUMBER_SIGNIFICAND_BYTES, sizeof(c.val));
	
	return c;
 }
