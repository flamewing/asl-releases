/* int_type.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* ASL                                                                       */
/*                                                                           */
/* Integer Types of various lengths and signedness                           */
/*                                                                           */
/*****************************************************************************/

#include "int_type.h"

/*!------------------------------------------------------------------------
 * \fn     as_log2(unsigned arg)
 * \brief  compute log2(arg)
 * \param  arg
 * \return log2 if arg is a power of 2, otherwise zero
 * ------------------------------------------------------------------------ */

unsigned as_log2(unsigned arg) {
    int ret = 0;
    while (arg) {
        if (arg & 1) {
            return (arg & ~1) ? 0 : ret;
        }
        ret++;
        arg >>= 1;
    }
    return 0;
}
