#include "QFloat.h"

QFloat operator * (const QFloat& a, const QFloat& b) {
	/* handle denormalized floating point */

	bool sign_a = (a.se >> NUMBER_EXPONENT_BITS);
	uint8_t exponent_a = a.se ^ (sign_a << NUMBER_EXPONENT_BITS);
	bool sign_b = (b.se >> NUMBER_EXPONENT_BITS);
	uint8_t exponent_b = b.se ^ (sign_b << NUMBER_EXPONENT_BITS);

	QFloat c;

	bool sign_c = sign_a ^ sign_b;
	uint8_t exponent_c = exponent_a + exponent_b - K;

	/* (1+x) * (1+y) = 1 + x + y + x*y */

	uint16_t tmp;
	int8_t n = 1;

	/* x*y */
	for (int i = 0; i < NUMBER_SIGNIFICAND_BYTES; ++i)
			for (int j = 0; j < NUMBER_SIGNIFICAND_BYTES; ++j) {
					int k = i + j - NUMBER_SIGNIFICAND_BYTES;

					tmp = (uint16_t)a.val[i] * b.val[j];
					for (; k < NUMBER_SIGNIFICAND_BYTES; ++k) {
						tmp += c.val[k];
						c.val[k] = tmp & UINT8_MAX;
						tmp >>= 8;
					}
			}

	/* x + y */
	tmp = 0;
	for (int i = 0; i < NUMBER_SIGNIFICAND_BYTES; ++i) {
		tmp += (int16_t)a.val[i] + b.val[i] + c.val[i];
		c.val[i] = tmp & UINT8_MAX;
		tmp >>= 8;
	}
	n += tmp;

	if (n > 1) {
		/* significand shift right 1 */
		bool l_bit = (n & 1), r_bit;
		for (int k = 0; k < NUMBER_SIGNIFICAND_BYTES; ++k) {
			r_bit = (c.val[k] & 1);
			c.val[k] = (c.val[k] >> 1) | (l_bit << 8);
			l_bit = r_bit;
		}
		++exponent_c;
	}

	c.se = exponent_c | (sign_c << NUMBER_EXPONENT_BITS);
	return c;
}