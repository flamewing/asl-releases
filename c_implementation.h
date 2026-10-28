/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#ifndef AS_C_IMPLEMENTATION_H
#define AS_C_IMPLEMENTATION_H

/* Include after the implementation's headers. Headers retain C++ compatibility
   diagnostics; C-only implementation code may use C's type and enum rules. */
#if defined(__GNUC__) && !defined(__cplusplus)
#    pragma GCC diagnostic ignored "-Wc++-compat"
#endif

#endif
