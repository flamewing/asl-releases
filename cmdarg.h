#ifndef CMDARG_H
#define CMDARG_H
/* cmdarg.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Verarbeitung Kommandozeilenparameter                                      */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*            2001-10-20: option string is pointer instead of array          */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "stringlists.h"

typedef enum {
    CMDErr,
    CMDFile,
    CMDOK,
    CMDArg
} CMDResult;

typedef CMDResult (*CMDCallback)(Boolean NegFlag, char const* Arg);
typedef void (*CMDErrCallback)(Boolean InEnv, char* Arg);

typedef struct {
    char const* Ident;
    CMDCallback Callback;
} CMDRec;

#define MAXPARAM 256
typedef Boolean CMDProcessed[MAXPARAM + 1];

extern StringList FileArgList;

extern Boolean ProcessedEmpty(CMDProcessed Processed);

extern void ProcessCMD(
        int argc, char** argv, CMDRec const* pCMDRecs, int CMDRecCnt,
        CMDProcessed Unprocessed, char const* EnvName, CMDErrCallback ErrProc);

extern char const* GetEXEName(char const* argv0);

extern void cmdarg_init(char* ProgPath);

#endif /* CMDARG_H */
