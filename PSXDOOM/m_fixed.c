/* Fixed_t.c  */

#include "doomdef.h"

fixed_t FixedMul(register fixed_t	a, register fixed_t	b)//L8003EEA4()
{
	register int             sign;
	register unsigned    	c;

	sign = a^b;

	if (a <= 0)
		a = -a;

	if (b <= 0)
		b = -b;

    //ASM CODE
	__asm__("multu	%0, %1;"    \
			"mfhi   %2;"		\
			"mflo   %3;"		\
			: "=r"(a),			\
			  "=r"(b)			\
			: "r" (a),			\
			  "r" (b));

	c = (a << 16) + (b >> 16);
    if (sign < 0)
		c = -c;

	return c;
}

fixed_t FixedDiv(register fixed_t a, register fixed_t b)//L8003EEF0()
{
	register unsigned        c;
	register unsigned        bit;
	register int             sign;

	sign = a^b;

	if (a <= 0)
		a = -a;

	if (b <= 0)
		b = -b;

	bit = 0x10000;
	do
	{
		b <<= 1;
		bit <<= 1;
	} while (b < a);

	c = 0;
	do
	{
		if (a >= b)
		{
			a -= b;
			c |= bit;
		}
		a <<= 1;
		bit >>= 1;
	} while (bit && a);

	if (sign < 0)
		c = -c;

	return c;
}
