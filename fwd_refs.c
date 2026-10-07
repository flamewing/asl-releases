/* fwd_refs.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* ASL                                                                       */
/*                                                                           */
/* Store and resolve symbol forward references for the purpose of symbol     */
/* usage tracking                                                            */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "fwd_refs.h"

#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "strutil.h"
#include "ulib/hash_open_prot.h"

/* A name can have references in several sections or macro invocations.
   Resolution visits all of that name's scopes, without sorted traversal. */
typedef struct as_forward_ref_entry {
    struct as_forward_ref_entry* next;
    LongInt attribute;
    Boolean resolved;
} as_forward_ref_entry_t;

static oh_size_t forward_ref_hash(char const* name) {
    oh_size_t hash = 5381;
    unsigned char const* p;
    for (p = (unsigned char const*)name; *p; ++p) {
        hash = hash * 33u + *p;
    }
    return hash;
}

static int forward_ref_equal(char const* a, char const* b) {
    return !strcmp(a, b);
}

DEFINE_OPENHASH(forwardrefs, char const*, as_forward_ref_entry_t*, 1,
                forward_ref_hash, forward_ref_equal)

static openhash_t(forwardrefs) forward_refs;

#define DBG_FORWARD_REF 0
#if DBG_FORWARD_REF
#    define fwd_ref_dbg_printf printf
#else
static int fwd_ref_dbg_printf(char const* fmt, ...) {
    UNUSED(fmt);
    return 0;
}
#endif

void as_forward_ref_add(char const* p_expanded_name) {
    LongInt attribute = (MomLocHandle >= 0) ? MomLocHandle : MomSectionHandle;
    as_forward_ref_entry_t* entry;
    as_forward_ref_entry_t** link;
    int inserted;
    oh_iter_t it = openhash_set(forwardrefs, &forward_refs, p_expanded_name,
                                &inserted);
    if (it == openhash_end(&forward_refs)) {
        abort();
    }
    if (inserted != OH_INS_ERR) {
        /* The insertion probe borrows the caller's name; only a new name
           receives owned storage. The bucket owns the name across all scopes. */
        char* name = as_strdup(p_expanded_name);
        if (!name) {
            abort();
        }
        openhash_key(&forward_refs, it) = name;
        openhash_value(&forward_refs, it) = NULL;
    }
    /* Local handles normally increase between macro invocations. Descending
       order makes a fresh handle or a duplicate of the current one a head
       operation, while retaining arbitrary section/handle insertion order. */
    link = &openhash_value(&forward_refs, it);
    while (*link && (*link)->attribute > attribute) {
        link = &(*link)->next;
    }
    if (*link && (*link)->attribute == attribute) {
        return;
    }
    entry = (as_forward_ref_entry_t*)calloc(1, sizeof(*entry));
    if (!entry) {
        abort();
    }
    entry->attribute = attribute;
    entry->next = *link;
    *link = entry;
    fwd_ref_dbg_printf("add forward_ref_entry %s/%ld\n", p_expanded_name,
                       (long)attribute);
}

static void clear_forward_refs(void) {
    oh_iter_t it;
    for (it = openhash_begin(&forward_refs); it < openhash_end(&forward_refs); ++it) {
        if (openhash_exist(&forward_refs, it)) {
            as_forward_ref_entry_t* entry = openhash_value(&forward_refs, it);
            while (entry) {
                as_forward_ref_entry_t* next = entry->next;
                free(entry);
                entry = next;
            }
            free((void*)openhash_key(&forward_refs, it));
        }
    }
    free(forward_refs.flags);
    free(forward_refs.keys);
    free(forward_refs.vals);
    memset(&forward_refs, 0, sizeof(forward_refs));
}

/*!------------------------------------------------------------------------
 * \fn     as_forward_ref_search_and_mark(const char *p_name, LongInt attribute)
 * \brief  search list of forward refs for entries matching freshly defined symbols
 * \param  p_name symbol's name
 * \param  attribute's macro local handle
 * \return True if any references existed
 * ------------------------------------------------------------------------ */

Boolean as_forward_ref_search_and_mark(char const* p_name, LongInt attribute) {
    as_forward_ref_entry_t* p_entry;
    Boolean ret = False;

    fwd_ref_dbg_printf(
            "search_and_mark_forward_refs '%s' defined in %ld:\n", p_name,
            (long)attribute);
    oh_iter_t it = openhash_get(forwardrefs, &forward_refs, p_name);
    if (it == openhash_end(&forward_refs)) {
        return False;
    }
    p_entry = openhash_value(&forward_refs, it);
    /* A macro-local definition can resolve only its own handle. There is
       no need to visit other scopes once the ordered search reaches it. */
    if (attribute >= LOC_HANDLE_OFFSET) {
        while (p_entry && p_entry->attribute > attribute) {
            p_entry = p_entry->next;
        }
        if (p_entry && p_entry->attribute == attribute && !p_entry->resolved) {
            p_entry->resolved = True;
            return True;
        }
        return False;
    }
    for (; p_entry; p_entry = p_entry->next) {
        fwd_ref_dbg_printf(" forward from %ld:\n", (long)p_entry->attribute);

        if (p_entry->resolved) {
            fwd_ref_dbg_printf(" ->already resolved\n");
            continue;
        }

        /* (1) The symbol is neither in a section, nor in a macro body.
               It matches all references: */

        if (attribute == -1) {
            p_entry->resolved = ret = True;
        }

        /* (2) The symbol is in a section.  It is visible for references
               from the same section, or from one of its subsections.
               It should also be visible from macro bodies inside the
               same or a sub section, but we do not yet have this
               information: */

        else {
            if ((p_entry->attribute == attribute)
                || is_sub_section(p_entry->attribute, attribute)) {
                p_entry->resolved = ret = True;
            }
        }
        fwd_ref_dbg_printf(" -> %sresolved\n", p_entry->resolved ? "" : "not ");
    }
    return ret;
}

/*!------------------------------------------------------------------------
 * \fn     as_forward_ref_init(void)
 * \brief  module initialization
 * ------------------------------------------------------------------------ */

void as_forward_ref_init(void) {
    add_exit_pass_proc(clear_forward_refs);
}
