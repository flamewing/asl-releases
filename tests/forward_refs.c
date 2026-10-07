/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "strutil.h"
#include "trees.h"

/* Instrument the real forward-reference implementation, not a duplicate
   model. Tree traversal itself uses the production trees.c implementation. */
static unsigned root_searches;
static TTree* counted_find_tree_first(TTree* tree, char const* name,
                                     as_tree_path_t* path) {
    root_searches++;
    return find_tree_first(tree, name, path);
}
#define find_tree_first counted_find_tree_first
#include "../fwd_refs.c"
#undef find_tree_first

LongInt MomLocHandle = -1, MomSectionHandle = -1;
static SimpProc pass_cleanup;

void add_exit_pass_proc(SimpProc proc) {
    pass_cleanup = proc;
}

/* Section 2 is a child of section 1; section 3 is unrelated. */
Boolean is_sub_section(LongInt child, LongInt parent) {
    return child == 2 && parent == 1;
}

char* as_strdup(char const* text) {
    size_t length = strlen(text) + 1;
    char* result = (char*)malloc(length);
    if (!result) {
        abort();
    }
    memcpy(result, text, length);
    return result;
}

static int mark(char const* name, LongInt attribute, Boolean expected) {
    unsigned before = root_searches;
    Boolean actual = as_forward_ref_search_and_mark(name, attribute);
    if (actual != expected || root_searches != before + 1) {
        fprintf(stderr, "Wrong resolution or root-search count for %s/%ld\n",
                name, (long)attribute);
        return 1;
    }
    return 0;
}

static void add(char const* name, LongInt section, LongInt local) {
    MomSectionHandle = section;
    MomLocHandle = local;
    as_forward_ref_add(name);
}

static int run(Boolean balanced) {
    BalanceTrees = balanced;
    if (mark("absent", -1, False)) return 1;
    add("same", 1, -1);
    add("same", 2, -1);
    add("same", 3, -1);
    add("same", 1, LOC_HANDLE_OFFSET);
    add("same", 1, LOC_HANDLE_OFFSET + 1);
    add("other", -1, -1);
    if (mark("missing", -1, False) || mark("same", 1, True)
        || mark("same", 1, False) || mark("same", 3, True)
        || mark("same", LOC_HANDLE_OFFSET + 2, False)
        || mark("same", LOC_HANDLE_OFFSET, True)
        || mark("same", LOC_HANDLE_OFFSET, False)
        || mark("same", LOC_HANDLE_OFFSET + 1, True)
        || mark("same", -1, False) || mark("other", -1, True)
        || mark("other", -1, False)) return 1;
    /* Repeated insertion must not unresolve an existing entry. */
    add("same", 1, -1);
    if (mark("same", 1, False)) return 1;
    add("global", 3, -1);
    add("global", 1, LOC_HANDLE_OFFSET);
    if (mark("global", -1, True) || mark("global", -1, False)) return 1;
    pass_cleanup();
    if (p_forward_ref_tree || mark("same", -1, False)) return 1;
    add("same", 1, -1);
    if (mark("same", 1, True)) return 1;
    pass_cleanup();
    return 0;
}

int main(void) {
    as_forward_ref_init();
    return !pass_cleanup || run(False) || run(True);
}
