#ifndef ASMLIST_H
#define ASMLIST_H
/* asmlist.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS Port                                                                   */
/*                                                                           */
/* Generate Listing                                                          */
/*                                                                           */
/*****************************************************************************/

extern void MakeList(char const* pSrcLine);

extern void as_list_set_max_pc(LargeWord max_pc);

extern void asmlist_setup(void);

extern void asmlist_init(void);

#endif /* ASMLIST_H */
