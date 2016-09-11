#ifndef _ASMERR_H
#define _ASMERR_H
/* asmerr.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Error Handling Functions                                                  */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmerr.h,v 1.2 2016/08/30 09:53:46 alfred Exp $                      */
/*****************************************************************************
 * $Log: asmerr.h,v $
 * Revision 1.2  2016/08/30 09:53:46  alfred
 * - make string argument const
 *
 * Revision 1.1  2008/01/02 22:32:21  alfred
 * - better heap checking for DOS target
 *
 *****************************************************************************/

extern void WrErrorString(char *Message, char *Add, Boolean Warning, Boolean Fatal);

extern void WrError(Word Num);

extern void WrXError(Word Num, const char *Message);

#endif /* _ASMERR_H */

