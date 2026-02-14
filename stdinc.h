#ifndef STDINC_H
#define STDINC_H
/* stdinc.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* globaler Einzug immer benoetigter includes                                */
/*                                                                           */
/* Historie: 21. 5.1996 min/max                                              */
/*           11. 5.1997 DOS-Anpassungen                                      */
/*                                                                           */
/*****************************************************************************/

#include "config.h"

#include <stddef.h>
#include <stdio.h>     // IWYU pragma: export
#include <stdlib.h>    // IWYU pragma: export
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif

#include <errno.h>    // IWYU pragma: export
#include <math.h>     // IWYU pragma: export
#include <memory.h>
#include <sys/types.h>
#if !defined(__FreeBSD__) && !defined(__APPLE__)
#    include <malloc.h>
#endif

#include "chardefs.h" // IWYU pragma: export
#include "datatypes.h" // IWYU pragma: export

#ifndef False
#    define False 0
#    define True  1
#endif

#define Ord(b) ((b) ? 1L : 0L)

#ifndef min
#    define min(a, b) ((a < b) ? (a) : (b))
#endif
#ifndef max
#    define max(a, b) ((a > b) ? (a) : (b))
#endif

#define as_array_size(a) (sizeof(a) / sizeof(*(a)))

#define as_abort(msg)       \
    do {                    \
        fputs(msg, stderr); \
        abort();            \
    } while (0)

#ifndef M_PI
#    define M_PI 3.1415926535897932385E0
#endif

#endif /* STDINC_H */
