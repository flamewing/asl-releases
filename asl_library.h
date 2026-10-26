/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#ifndef ASL_LIBRARY_H
#define ASL_LIBRARY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Assemble using CLI arguments, including argv[0]. Arguments are borrowed and
   never modified. Successive calls reset invocation options and CPU aliases.
   Calls use process-wide assembler state and must be serialized by the caller;
   recursive entry returns 5. Diagnostics use the usual CLI streams/options.
   Return codes match the CLI: 0 success, 1 no input, 2 assembly errors,
   3 fatal assembly error, 4 invalid arguments, 255 initialization failure.
   Initialization tables and reusable buffers remain cached for process life. */
int asl_assemble(int argc, char const* const argv[]);

#ifdef __cplusplus
}
#endif

#endif
