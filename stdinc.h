#ifndef _STDINC_H
#define _STDINC_H
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

#include <stddef.h>
#include <stdio.h>
#ifndef __MUNIX__
#include <stdlib.h>
#endif
#if !defined ( __MSDOS__ ) && !defined( __IBMC__ ) && !defined( _MSC_VER )
#include <unistd.h>
#endif
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#ifdef __MSDOS__
#include <alloc.h>
#else
#include <memory.h>
#if !defined (__FreeBSD__) && !defined(__APPLE__)
#include <malloc.h>
#endif
#endif

#include "pascstyle.h"
#include "datatypes.h"
#include "chardefs.h"

#ifndef min
#define min(a,b) ((a<b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a>b)?(a):(b))
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932385E0
#endif

#endif /* _STDINC_H */
