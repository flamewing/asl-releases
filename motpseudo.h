#ifndef _MOTPSEUDO_H
#define _MOTPSEUDO_H
/* motpseudo.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Motorola-Pseudo-Befehle                                */
/*                                                                           */
/*****************************************************************************/
/* $Id: motpseudo.h,v 1.1 2004/05/29 12:04:48 alfred Exp $                   */
/*****************************************************************************
 * $Log: motpseudo.h,v $
 * Revision 1.1  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/
  
/*****************************************************************************
 * Global Functions
 *****************************************************************************/
    
extern Boolean DecodeMotoPseudo(Boolean Turn);

extern void ConvertMotoFloatDec(Double F, Word *w);

extern void AddMoto16PseudoONOFF(void);

extern Boolean DecodeMoto16Pseudo(ShortInt OpSize, Boolean Turn);

#endif /* _MOTPSEUDO_H */
