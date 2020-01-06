/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
#ifndef _INTCONSTS_H
#define _INTCONSTS_H

#ifdef __STDC__

#define INTCONST_ff000000 0xff000000u
#define INTCONST_ffff0000 0xffff0000u
#define INTCONST_ffffff00 0xffffff00u
#define INTCONST_fffffff0 0xfffffff0u
#define INTCONST_fffffffe 0xfffffffeu
#define INTCONST_ffffffff 0xffffffffu
#define INTCONST_ffffffffl 0xfffffffflu
#define INTCONST_f8ffff00 0xf8ffff00u

#define INTCONST_9223372036854775807 9223372036854775807ull
#define INTCONST_4294967295 4294967295ul

#else /* !__STDC__ */

#define INTCONST_ff000000 0xff000000
#define INTCONST_ffff0000 0xffff0000
#define INTCONST_ffffff00 0xffffff00
#define INTCONST_ffffff00 0xffffff00
#define INTCONST_fffffff0 0xfffffff0
#define INTCONST_fffffffe 0xfffffffe
#define INTCONST_ffffffff 0xffffffff
#define INTCONST_ffffffffl 0xffffffffl
#define INTCONST_f8ffff00 0xf8ffff00

#define INTCONST_9223372036854775807 9223372036854775807
#define INTCONST_4294967295 4294967295

#endif /* __STDC__ */

#endif /* _INTCONSTS_H */
