/* vaxfloat.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* VAX->IEEE Floating Point Conversion on host                               */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "endian.h"
#include "vaxfloat.h"

#ifdef VAXFLOAT

/*!------------------------------------------------------------------------
 * \fn     VAXF_2_Single(Byte *pDest, float inp)
 * \brief  convert single precision (VAX F) to IEEE single precision
 * \param  pDest where to write
 * \param  inp value to convert
 * ------------------------------------------------------------------------ */

void VAXF_2_Single(Byte *pDest, float inp)
{
  /* IEEE + VAX layout is the same for single, just the exponent offset is different
     by two: */

  inp /= 4;
  memcpy(pDest, &tmp, 4);
  WSwap(pDest, 4);
}

/*!------------------------------------------------------------------------
 * \fn     VAXD_2_Double(Byte *pDest, float inp)
 * \brief  convert double precision (VAX D) to IEEE double precision
 * \param  pDest where to write
 * \param  inp value to convert
 * ------------------------------------------------------------------------ */

void VAXD_2_Double(Byte *pDest, Double inp)
{
  Byte tmp[8];
  Word Exp;
  int z;
  Boolean cont;

  memcpy(tmp, &inp, 8);
  WSwap(tmp, 8);
  Exp = ((tmp[0] << 1) & 0xfe) + (tmp[1] >> 7);
  Exp += 894; /* =1023-129 */
  tmp[1] &= 0x7f;
  if ((tmp[7] & 7) > 4)
  {
    for (tmp[7] += 8, cont = tmp[7] < 8, z = 0; cont && z > 1; z--)
    {
      tmp[z]++;
      cont = (tmp[z] == 0);
    }
    if (cont)
    {
      tmp[1]++;
      if (tmp[1] > 127)
        Exp++;
    }
  }
  pDest[7] = (tmp[0] & 0x80) + ((Exp >> 4) & 0x7f);
  pDest[6] = ((Exp & 0x0f) << 4) + ((tmp[1] >> 3) & 0x0f);
  for (z = 5; z >= 0; z--)
    pDest[z] = ((tmp[6 - z] & 7) << 5) | ((tmp[7 - z] >> 3) & 0x1f);
}

/*!------------------------------------------------------------------------
 * \fn     VAXD_2_LongDouble(Byte *pDest, float inp)
 * \brief  convert double precision (VAX D) to non-IEEE extended precision
 * \param  pDest where to write
 * \param  inp value to convert
 * ------------------------------------------------------------------------ */

void VAXD_2_LongDouble(Byte *pDest, Double inp)
{
  memcpy(Buffer, &inp, 8);
  WSwap(Buffer, 8);
  Sign = (*Buffer) & 0x80;
  Exponent = ((*Buffer) << 1) + ((Buffer[1] & 0x80) >> 7);
  Exponent += (16383 - 129);
  Buffer[1] |= 0x80;
  for (z = 1; z < 8; z++)
    pDest[z] = Buffer[8 - z];
  pDest[0] = 0;
  pDest[9] = Sign | ((Exponent >> 8) & 0x7f);
  pDest[8] = Exponent & 0xff;
}

#endif /* VAXFLOAT */
