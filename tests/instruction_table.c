/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"

#include "asmitree.h"
#include "strutil.h"

#include <string.h>

#include "c_implementation.h"

static unsigned calls;

static void instruction(Word index) {
    calls = calls * 10 + index;
}

static Boolean function(Word index) {
    calls = index;
    return True;
}

int main(void) {
    static char const borrowed[] = "BORROWED";
    char              owned[]    = "OWNED";
    PInstTable        table      = CreateInstTable(101);
    inst_fnc_table_t* functions  = inst_fnc_table_create(101);
    int               failed     = 0;
    strutil_init();
    inst_table_set_prefix_proc(table, instruction, 1);
    AddInstTable(table, borrowed, 2, instruction);
    failed |= inst_table_search(table, borrowed)->Name != borrowed;
    failed |= inst_table_search(table, borrowed)->OwnedName != NULL;
    SetDynamicInstTable(table);
    AddInstTable(table, owned, 3, instruction);
    owned[0] = 'X';
    failed |= !LookupInstTable(table, "OWNED") || calls != 13;
    calls = 0;
    failed |= !LookupInstTable(table, borrowed) || calls != 12;
    RemoveInstTable(table, "OWNED");
    failed |= inst_table_search(table, "OWNED") != NULL || table->Fill != 1;
    DestroyInstTable(table);

    inst_fnc_table_add(functions, borrowed, 4, function);
    functions->dynamic = True;
    strcpy(owned, "OWNED");
    inst_fnc_table_add(functions, owned, 5, function);
    owned[0] = 'X';
    failed |= !inst_fnc_table_lookup(functions, "OWNED") || calls != 5;
    failed |= !inst_fnc_table_lookup(functions, borrowed) || calls != 4;
    failed |= inst_fnc_table_lookup(functions, "MISSING");
    inst_fnc_table_destroy(functions);
    DestroyInstTable(NULL);
    inst_fnc_table_destroy(NULL);
    if (failed) {
        fprintf(stderr, "Instruction table name ownership changed\n");
    }
    return failed;
}
