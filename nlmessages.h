#ifndef NLMESSAGES_H
#define NLMESSAGES_H
/* nlmessages.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Einlesen und Verwalten von Meldungs-Strings                               */
/*                                                                           */
/* Historie: 13. 8.1997 Grundsteinlegung                                     */
/*           17. 8.1997 Verallgemeinerung auf mehrere Kataloge               */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

#include <stddef.h>

typedef struct {
    char*    MsgBlock;
    LongInt *StrPosis, MsgCount;
} TMsgCat, *PMsgCat;

/* Borrowed NUL-terminated strings from an initialized, validated catalog.
   Valid-index pointers live with the catalog. Invalid indices use one shared
   static buffer overwritten by the next invalid-index lookup. Do not free. */
extern char* catgetmessage(PMsgCat Catalog, int Num);

extern void msg_catalog_open_file(
        PMsgCat p_catalog, char const* p_file_name, char const* p_exe_path,
        LongInt file_msg_id1, LongInt file_msg_id2);
extern void msg_catalog_open_buffer(
        PMsgCat p_catalog, unsigned char const* p_buffer, size_t buffer_size,
        LongInt file_msg_id1, LongInt file_msg_id2);

extern char* getmessage(int Num);

extern void nlmessages_init_file(
        char const* p_file_name, char* p_exe_path, LongInt msg_id1, LongInt msg_id2);
extern void nlmessages_init_buffer(
        unsigned char const* p_buffer, size_t buffer_size, LongInt msg_id1,
        LongInt msg_id2);

#endif /* NLMESSAGES_H */
