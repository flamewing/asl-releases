/* stdinc.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* globaler Einzug immer benoetigter includes                                */
/*                                                                           */
/* Historie: 21. 5.1996 min/max                                              */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>
#include <memory.h>
#include <malloc.h>

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
