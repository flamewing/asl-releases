#ifndef _ASMIF_H
#define _ASMIF_H
/* asmif.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Befehle zur bedingten Assemblierung                                       */
/*                                                                           */
/* Historie: 15. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmif.h,v 1.2 2002/05/01 15:56:09 alfred Exp $                       */
/***************************************************************************** 
 * $Log: asmif.h,v $
 * Revision 1.2  2002/05/01 15:56:09  alfred
 * - print start line of IF/SWITCH construct when it ends
 *
 *
 *****************************************************************************/
     
typedef struct _TIfSave
         {
	  struct _TIfSave *Next;
	  Integer NestLevel;
	  Boolean SaveIfAsm;
	  TempResult SaveExpr;
	  enum {IfState_IFIF,IfState_IFELSE,
		   IfState_CASESWITCH,IfState_CASECASE,IfState_CASEELSE} State;
	  Boolean CaseFound;
	  LongInt StartLine;
         } TIfSave,*PIfSave;


extern Boolean IfAsm;
extern PIfSave FirstIfSave;


extern Boolean CodeIFs(void);

extern void AsmIFInit(void);

extern Integer SaveIFs(void);

extern void RestoreIFs(Integer Level);

extern Boolean IFListMask(void);

extern void asmif_init(void);
#endif /* _ASMIF_H */
