/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "strutil.h"
#include "trees.h"

/* Include the real implementation and count scope/name allocations. Hash
   collisions, growth and cleanup exercise the production table implementation. */
static unsigned entry_allocations, name_allocations;
static void* counted_calloc(size_t count, size_t size) {
    entry_allocations++;
    return calloc(count, size);
}
#define calloc counted_calloc
#include "../fwd_refs.c"
#undef calloc

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
    name_allocations++;
    char* result = (char*)malloc(length);
    if (!result) {
        abort();
    }
    memcpy(result, text, length);
    return result;
}

static int mark(char const* name, LongInt attribute, Boolean expected) {
    Boolean actual = as_forward_ref_search_and_mark(name, attribute);
    if (actual != expected) {
        fprintf(stderr, "Wrong resolution for %s/%ld\n",
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

static int allocation_checks(void) {
    unsigned order[1024], i, repeat, entries = entry_allocations;
    unsigned names = name_allocations, seed = 0x12345678u;
    char name[40];
    for (i = 0; i < 1024; i++) order[i] = i;
    for (i = 1023; i; i--) {
        unsigned index, tmp;
        seed = seed * 1664525u + 1013904223u;
        index = seed % (i + 1);
        tmp = order[i]; order[i] = order[index]; order[index] = tmp;
    }
    for (i = 0; i < 1024; i++) {
        snprintf(name, sizeof(name), "entry%04u", order[i]);
        add(name, 1, -1);
    }
    if (entry_allocations != entries + 1024 || name_allocations != names + 1024) {
        fprintf(stderr, "Unique references must allocate one entry and name each\n");
        return 1;
    }
    entries = entry_allocations; names = name_allocations;
    for (repeat = 0; repeat < 16; repeat++) {
        for (i = 0; i < 1024; i++) {
            snprintf(name, sizeof(name), "entry%04u", order[i]);
            add(name, 1, -1);
            if (mark(name, 1, repeat == 0)) return 1;
        }
    }
    if (entry_allocations != entries || name_allocations != names) {
        fprintf(stderr, "Duplicate references must not allocate entries or names\n");
        return 1;
    }
    if (mark("Ab", 1, True) || mark("BA", 1, True)) return 1;
    pass_cleanup();
    return 0;
}

static int collision_and_scope_checks(void) {
    unsigned i, entries, names;
    /* These distinct names have the same complete djb2 hash, not just the
       same initial bucket. They must survive equality checks and resizing. */
    if (forward_ref_hash("Ab") != forward_ref_hash("BA")) return 1;
    add("Ab", 1, -1);
    add("BA", 1, -1);
    if (allocation_checks()) return 1;
    /* The stress helper cleared the pass; rebuild the colliding pair. */
    add("Ab", 1, -1);
    add("BA", 1, -1);
    if (mark("Ab", 1, True) || mark("BA", 1, True)
        || mark("Ab", 1, False) || mark("BA", 1, False)) return 1;
    entries = entry_allocations; names = name_allocations;
    for (i = 0; i < 128; ++i) {
        add("scoped", -1, LOC_HANDLE_OFFSET + ((i * 73u) % 128u));
    }
    if (entry_allocations != entries + 128 || name_allocations != names + 1) {
        fprintf(stderr, "Scopes must share one owned name, with distinct records\n");
        return 1;
    }
    entries = entry_allocations; names = name_allocations;
    for (i = 0; i < 128; ++i) {
        add("scoped", -1, LOC_HANDLE_OFFSET + i);
        if (mark("scoped", LOC_HANDLE_OFFSET + i, True)) return 1;
    }
    if (entry_allocations != entries || name_allocations != names
        || mark("scoped", -1, False)) return 1;
    add("scoped", 3, -1);
    if (mark("scoped", -1, True) || mark("scoped", -1, False)) return 1;
    pass_cleanup();
    if (forward_refs.nbucket || forward_refs.nelem) return 1;
    return mark("Ab", -1, False) || mark("BA", -1, False);
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
    if (openhash_size(&forward_refs) || forward_refs.nbucket || mark("same", -1, False)) return 1;
    add("same", 1, -1);
    if (mark("same", 1, True)) return 1;
    pass_cleanup();
    return collision_and_scope_checks();
}

int main(void) {
    as_forward_ref_init();
    return !pass_cleanup || run(False) || run(True);
}
