/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"

#include "cpulist.h"
#include "strutil.h"

#include <string.h>

#include "c_implementation.h"

static int  selected, freed, lines;
static char listing[1024];

static void plain_switch(void) {
    selected = 100;
}

static void owned_switch(void* data) {
    selected = ++*(int*)data;
}

static void owned_free(void* data) {
    freed++;
    free(data);
}

static void const_switch(void const* data) {
    selected = *(int const*)data;
}

static void other_const_switch(void const* data) {
    selected = 10 + *(int const*)data;
}

static void print_line(char const* text) {
    strmaxcat(listing, text, sizeof(listing));
    strmaxcat(listing, "\n", sizeof(listing));
    if (*text) {
        lines++;
    }
}

int main(void) {
    static int const properties[] = {1, 2};
    int*             owned        = (int*)malloc(sizeof(*owned));
    CPUVar           plain, mutable, first, second, other;
    int              failed = 0;
    if (!owned) {
        return 1;
    }
    *owned = 40;
    strutil_init();
    cpulist_init();
    plain   = AddCPU("PLAIN", plain_switch);
    mutable = AddCPUUser("OWNED", owned_switch, owned, owned_free);
    first   = AddCPUConstUser("CONST_A", const_switch, &properties[0]);
    second  = AddCPUConstUser("CONST_B", const_switch, &properties[1]);
    other   = AddCPUConstUser("OTHER", other_const_switch, &properties[0]);
    failed |= !AddCPUAlias("CONST_A", "CONST_ALIAS");
    failed |= !AddCPUAlias("OWNED", "OWNED_ALIAS");
    SwitchCPU(LookupCPUDefByVar(plain));
    failed |= selected != 100;
    SwitchCPU(LookupCPUDefByVar(mutable));
    failed |= selected != 41;
    SwitchCPU(LookupCPUDefByVar(first));
    failed |= selected != 1;
    SwitchCPU(LookupCPUDefByVar(second));
    failed |= selected != 2;
    SwitchCPU(LookupCPUDefByVar(other));
    failed |= selected != 11;
    SwitchCPU(LookupCPUDefByName("CONST_ALIAS"));
    failed |= selected != 1;
    SwitchCPU(LookupCPUDefByName("OWNED_ALIAS"));
    failed |= selected != 42;
    PrintCPUList(print_line);
    failed |= lines != 4 || !strstr(listing, "CONST_A") || !strstr(listing, "CONST_B")
              || strstr(listing, "CONST_ALIAS") != NULL;
    ClearCPUAliases();
    failed |= LookupCPUDefByName("CONST_ALIAS") != NULL || freed != 0;
    SwitchCPU(LookupCPUDefByVar(second));
    failed |= selected != 2;
    ClearCPUList();
    failed |= freed != 1;
    if (failed) {
        fprintf(stderr, "CPU callback ownership or grouping changed\n");
    }
    return failed;
}
