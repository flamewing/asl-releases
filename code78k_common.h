/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#ifndef CODE78K_COMMON_H
#define CODE78K_COMMON_H

/* A decoded ModMem operand contains one to three bytes. Its first byte is
   emitted separately; return the size of the remaining initialized prefix.
   Invalid counts indicate a decoder bug and terminate even in release builds. */
extern unsigned as_78k_mem_tail_count(int count);

#endif
