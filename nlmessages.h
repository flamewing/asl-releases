#ifndef _NLMESSAGES_H
#define _NLMESSAGES_H
/* nlmessages.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Einlesen und Verwalten von Meldungs-Strings                               */
/*                                                                           */
/* Historie: 13. 8.1997 Grundsteinlegung                                     */
/*           17. 8.1997 Verallgemeinerung auf mehrere Kataloge               */
/*                                                                           */
/*****************************************************************************/

typedef struct
         {
          char *MsgBlock;
          LongInt *StrPosis,MsgCount;
         } TMsgCat,*PMsgCat;

extern char *catgetmessage(PMsgCat Catalog, int Num);

extern void opencatalog(PMsgCat Catalog, char *File, char *Path, LongInt MsgId1, LongInt MsgId2);

extern char *getmessage(int Num);

extern void nlmessages_init(char *File, char *Path, LongInt MsgId1, LongInt MsgId2);
#endif /* _NLMESSAGES_H */
