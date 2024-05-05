#ifndef ASMERR_H
#define ASMERR_H
/* asmerr.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Error Handling Functions                                                  */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "errmsg.h"

extern Word ErrorCount, WarnCount;

struct sLineComp;
struct sStrComp;
extern void WrErrorString(
        char const* Message, char const* Add, Boolean Warning, Boolean Fatal,
        char const* pExtendError, const struct sLineComp* pLineComp);

extern void WrError(tErrorNum Num);

extern void WrXError(tErrorNum Num, char const* pExtError);

extern void WrXErrorPos(
        tErrorNum Num, char const* pExtError, const struct sLineComp* pLineComp);

extern void WrStrErrorPos(tErrorNum Num, const struct sStrComp* pStrComp);

extern void CodeEXPECT(Word Code);
extern void CodeENDEXPECT(Word Code);

extern void AsmErrPassInit(void);
extern void AsmErrPassExit(void);

extern void ChkIO(tErrorNum ErrNo);
extern void ChkXIO(tErrorNum ErrNo, char* pExtError);
extern void ChkStrIO(tErrorNum ErrNo, const struct sStrComp* pComp);

#endif /* ASMERR_H */
