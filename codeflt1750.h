/* flt1750.h  --  exports of flt1750.c */

extern double from_1750flt  (short  *input);  /* input: array of 2 shorts */
extern int      to_1750flt  (double input, short output[2]);
extern double from_1750eflt (short  *input);  /* input: array of 3 shorts */
extern int      to_1750eflt (double input, short output[3]);

