#ifndef MSG_LEVEL_H
#define MSG_LEVEL_H
/* msglevel.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* Message Level Handling                                                    */
/*                                                                           */
/*****************************************************************************/

#include "cmdarg.h"

enum {
    e_msg_level_quiet   = 0,
    e_msg_level_normal  = 1,
    e_msg_level_verbose = 2
};

extern unsigned msg_level;
extern ShortInt msg_quiet_depth;

extern void msg_level_init(void);

#endif /* MSG_LEVEL_H */
