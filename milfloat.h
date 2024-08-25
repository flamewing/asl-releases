#ifndef AS_MILFLOAT_H
#define AS_MILFLOAT_H
/* tifloat.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* IEEE -> MIL STD 1750 Floating Point Conversion on host                    */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

int as_float_2_mil1750(as_float_t inp, Word* p_dest, Boolean extended);

#endif /* AS_MILFLOAT_H */
