/***************************************************************************/
/*                                                                         */
/* Project   :        Mil-Std-1750 Host Development Support Library        */
/*                                                                         */
/* Component :   flt1750.c -- host independent float conversion routines   */
/*                                                                         */
/* Copyright :         (C) Daimler-Benz Aerospace AG, 1994-97              */
/*                                                                         */
/* Author    :      Oliver M. Kellogg, Dornier Satellite Systems,          */
/*                     Dept. RST13, D-81663 Munich, Germany.               */
/* Contact   :           oliver.kellogg@space.otn.dasa.de                  */
/*                                                                         */
/* Disclaimer:                                                             */
/*                                                                         */
/*  This program is free software; you can redistribute it and/or modify   */
/*  it under the terms of the GNU General Public License as published by   */
/*  the Free Software Foundation; either version 2 of the License, or      */
/*  (at your option) any later version.                                    */
/*                                                                         */
/*  This program is distributed in the hope that it will be useful,        */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*  GNU General Public License for more details.                           */
/*                                                                         */
/*  You should have received a copy of the GNU General Public License      */
/*  along with this program; if not, write to the Free Software            */
/*  Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.   */
/*                                                                         */
/***************************************************************************/

/* These conversion functions have been tested on MIPS(ULTRIX), SPARC(Solaris),
   i386(Linux/DOS), and VAX(VMS) machines.  Should you encounter any problems
   on other machines, please write to the e-mail contact address given above.
 */


#include "codeflt1750.h"
#include <math.h>
#define dfrexp     frexp
#define pow2(exp)  pow(2.0,(double)exp)

#define ushort unsigned short
#define ulong  unsigned long

#define FLOATING_TWO_TO_THE_FIFTEEN            32768.0
#define FLOATING_TWO_TO_THE_TWENTYTHREE      8388608.0
#define FLOATING_TWO_TO_THE_THIRTYONE     2147483648.0
#define FLOATING_TWO_TO_THE_THIRTYNINE  549755813888.0



double
from_1750flt (short *input)	/* input : array of 2 shorts */
{
  long int_mant;
  double flt_mant, flt_exp;
  signed char int_exp;

  int_exp = (signed char) (input[1] & 0xFF);
  int_mant = ((long) input[0] << 8) | (((long) input[1] & 0xFF00L) >> 8);
  /* printf("int_mant = 0x%08lx\n",int_mant); */
  flt_mant = (double) int_mant / FLOATING_TWO_TO_THE_TWENTYTHREE;
  flt_exp = pow2 (int_exp);
  return flt_mant * flt_exp;
}

int
to_1750flt (double input, short output[2])
{
  int exp;
  long mant;

  input = dfrexp (input, &exp);

  if (exp < -128)
    return -1;			/* signalize underflow */
  else if (exp > 127)
    return 1;			/* signalize overflow */

  if (input < 0.0 && input >= -0.5)	/* prompted by UNIX frexp */
    {
      input *= 2.0;
      exp--;
    }

  mant = (long) (input * FLOATING_TWO_TO_THE_THIRTYONE);

  /* printf("\n\tmant=%08lx\n",mant); */
  output[0] = (short) (mant >> 16);
  output[1] = (short) (mant & 0xFF00) | (exp & 0xFF);

  return 0;			/* success status */
}

double
from_1750eflt (short *input)	/* input : array of 3 shorts */
{
  long int_mant_hi, int_mant_lo;
  double flt_mant, flt_exp;
  signed char int_exp;

  int_exp = (signed char) (input[1] & 0xFF);

  int_mant_hi = (((long) input[0] << 16) | ((long) input[1] & 0xFF00L)) >> 8;
  int_mant_lo = ((long) input[2] & 0xFFFFL);

  flt_mant = (double) int_mant_hi / FLOATING_TWO_TO_THE_TWENTYTHREE
    + (double) int_mant_lo / FLOATING_TWO_TO_THE_THIRTYNINE;
  flt_exp = pow2 (int_exp);
/*  printf ("\tfrom: mant=%.12g, exp=%g\n", flt_mant, flt_exp);  */

  return flt_mant * flt_exp;
}

int
to_1750eflt (double input, short output[3])
{
  int exp;
  int is_neg = 0;
  ushort hlp;

  if (input < 0.0)
    {
      is_neg = 1;
      input = -input - .03125 / FLOATING_TWO_TO_THE_THIRTYNINE;
    }

  input = dfrexp (input, &exp);	/* input is now normalized mantissa */

  if (input == 1.0)		/* prompted by VAX frexp */
    {
      input = 0.5;
      exp++;
    }

  if (exp < -128)
    return -1;			/* signalize underflow */
  else if (exp > 127)
    return 1;			/* signalize overflow */

  output[0] = (short) (input * FLOATING_TWO_TO_THE_FIFTEEN);
  input -= (double) output[0] / FLOATING_TWO_TO_THE_FIFTEEN;
  hlp = (ushort) (input * FLOATING_TWO_TO_THE_TWENTYTHREE);
  output[1] = (short) ((hlp << 8) | (exp & 0xFF));
  input -= (double) hlp / FLOATING_TWO_TO_THE_TWENTYTHREE;
  hlp = (ushort) (input * FLOATING_TWO_TO_THE_THIRTYNINE);
  output[2] = (short) hlp;

  if (is_neg)
    {
      output[0] = ~output[0];
      output[1] = (~output[1] & 0xFF00) | (output[1] & 0x00FF);
      output[2] = ~output[2];
    }

  return 0;			/* success status */
}

