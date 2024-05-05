/* asminclist.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Include-Verschachtelungsliste                              */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "asminclist.h"

#include "as.rsc"
#include "asmdef.h"
#include "asmfnums.h"
#include "asmsub.h"
#include "nlmessages.h"
#include "strutil.h"

#include <string.h>

typedef struct sFileNode {
    Integer            Name;
    Integer            Len;
    struct sFileNode*  Parent;
    struct sFileNode** Subs;
} TFileNode, *PFileNode;
typedef struct sFileNode** PFileArray;

static PFileNode Root, Curr;

void PushInclude(char* S) {
    PFileNode Neu;

    Neu         = (PFileNode)malloc(sizeof(TFileNode));
    Neu->Name   = GetFileNum(S);
    Neu->Len    = 0;
    Neu->Subs   = NULL;
    Neu->Parent = Curr;
    if (!Root) {
        Root = Neu;
    }
    if (!Curr) {
        Curr = Neu;
    } else {
        if (Curr->Len == 0) {
            Curr->Subs = (PFileArray)malloc(sizeof(void*));
        } else {
            Curr->Subs = (PFileArray)realloc(Curr->Subs, sizeof(void*) * (Curr->Len + 1));
        }
        Curr->Subs[Curr->Len++] = Neu;
        Curr                    = Neu;
    }
}

void PopInclude(void) {
    if (Curr) {
        Curr = Curr->Parent;
    }
}

static void PrintIncludeList_PrintNode(PFileNode Node, int Indent) {
    int    z;
    String h;

    if (Node) {
        strmaxcpy(h, Blanks(Indent), STRINGSIZE);
        strmaxcat(h, GetFileName(Node->Name), STRINGSIZE);
        WrLstLine(h);
        for (z = 0; z < Node->Len; z++) {
            PrintIncludeList_PrintNode(Node->Subs[z], Indent + 5);
        }
    }
}

void PrintIncludeList(void) {
    NewPage(ChapDepth, True);
    WrLstLine(getmessage(Num_ListIncludeListHead1));
    WrLstLine(getmessage(Num_ListIncludeListHead2));
    WrLstLine("");
    PrintIncludeList_PrintNode(Root, 0);
}

static void ClearIncludeList_ClearNode(PFileNode Node) {
    int z;

    if (Node) {
        for (z = 0; z < Node->Len; ClearIncludeList_ClearNode(Node->Subs[z++]))
            ;
        if (Node->Len > 0) {
            free(Node->Subs);
        }
        free(Node);
    }
}

void ClearIncludeList(void) {
    ClearIncludeList_ClearNode(Root);
    Curr = NULL;
    Root = NULL;
}

void asminclist_init(void) {
    Root = NULL;
    Curr = NULL;
}
