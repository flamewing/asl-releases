/* version.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Lagert die Versionsnummer                                                 */
/*                                                                           */
/* Historie: 14.10.1997 Grundsteinlegung                                     */
/*           25. 7.1998 Version 1.41r8beta                                   */
/*           18.10.1998 Build 4                                              */
/*           25.10.1998 Build 5                                              */
/*           10. 1.1999 Build 6                                              */
/*           17. 1.1999 Build 7                                              */
/*           27. 1.1999 Build 8                                              */
/*           27. 1.1999 Build 9                                              */
/*            7. 2.1999 Build 10                                             */
/*           19. 4.1999 Build 11                                             */
/*           20. 4.1999 Build 12                                             */
/*            2. 5.1999 Build 13                                             */
/*            6. 7.1999 Build 14                                             */
/*           15. 9.1999 Build 15                                             */
/*            7.11.1999 Final Build                                          */
/*            9. 1.2000 Version 1.42beta                                     */
/*           13. 2.2000 Build 2                                              */
/*           13. 3.2000 Build 3                                              */
/*            4. 6.2000 Build 5                                              */
/*           18. 6.2000 Build 6                                              */
/*            6. 8.2000 Build 7                                              */
/*           20. 9.2000 Build 8                                              */
/*           15.10.2000 Build 9                                              */
/*            7. 1.2001 Build 10                                             */
/*           14. 1.2001 Build 11                                             */
/*            4. 2.2001 Build 12                                             */
/*           30. 5.2001 Build 13                                             */
/*            1. 7.2001 Build 14                                             */
/*           20. 8.2001 Build 15                                             */
/*            7.10.2001 Build 16                                             */
/*           2001-11-04 Build 17                                             */
/*           2002-01-13 Build 18                                             */
/*           2002-01-27 Build 19                                             */
/*                                                                           */
/*****************************************************************************/
/* $Id: version.c,v 1.49 2010/04/11 11:40:42 alfred Exp $                     */
/***************************************************************************** 
 * $Log: version.c,v $
 * Revision 1.49  2010/04/11 11:40:42  alfred
 * - 1.42 Bld 76
 *
 * Revision 1.48  2010/03/27 15:03:36  alfred
 * - 1.42 Bld75
 *
 * Revision 1.47  2010/03/14 11:05:59  alfred
 * - version 1.42 Bld74
 *
 * Revision 1.46  2010/02/27 14:25:19  alfred
 * - 1.42 Bld73
 *
 * Revision 1.45  2010/01/01 14:58:02  alfred
 * - 1.42 Bld72
 *
 * Revision 1.44  2009/09/05 23:14:09  alfred
 * - -
 *
 * Revision 1.43  2009/06/07 09:35:23  alfred
 * - 1.42 Bld70
 *
 * Revision 1.42  2009/05/10 10:50:48  alfred
 * - 1.42 Bld 69
 *
 * Revision 1.41  2009/04/13 07:37:26  alfred
 * - update version
 *
 * Revision 1.40  2009/04/10 09:41:41  alfred
 * - 1.42 Bld 67
 *
 * Revision 1.39  2009/02/08 12:53:44  alfred
 * - 1.42 Bld66
 *
 * Revision 1.38  2008/10/25 09:48:10  alfred
 * - 1.42 Bld 64
 *
 * Revision 1.37  2008/08/29 19:20:10  alfred
 * - 1.42 Bld 63
 *
 * Revision 1.36  2008/08/22 19:10:22  alfred
 * - 1.42 Bld62
 *
 * Revision 1.35  2008/08/17 11:39:08  alfred
 * 1.42 Bld60
 *
 * Revision 1.34  2008/08/10 12:00:30  alfred
 * 1.42 Bld60
 *
 * Revision 1.33  2008/06/22 11:01:12  alfred
 * - 1.42 Bld59
 *
 * Revision 1.32  2008/03/31 18:40:45  alfred
 * - -
 *
 * Revision 1.31  2008/03/31 18:31:12  alfred
 * - 1.42 Bld 58
 *
 * Revision 1.30  2007/12/31 13:39:30  alfred
 * - Bld57
 *
 * Revision 1.29  2007/09/24 17:53:01  alfred
 * - 1.42 Bld56
 *
 * Revision 1.28  2007/05/01 09:55:59  alfred
 * - 1.42 Bld 55
 *
 * Revision 1.27  2006/12/19 17:28:43  alfred
 * - 1.42 Bld54
 *
 * Revision 1.26  2006/12/17 12:07:36  alfred
 * - 1.52bld53
 *
 * Revision 1.25  2006/12/09 18:28:18  alfred
 * - 1.42 Bld52
 *
 * Revision 1.24  2006/08/26 11:10:26  alfred
 * - 1.42 Bld51
 *
 * Revision 1.23  2006/08/05 18:28:35  alfred
 * - 1.42 bld 50
 *
 * Revision 1.22  2006/06/24 19:09:06  alfred
 * 1.42 Bld49
 *
 * Revision 1.21  2006/05/08 15:39:33  alfred
 * - 1.42 bld48
 *
 * Revision 1.20  2006/04/15 11:12:17  alfred
 * - Bld47
 *
 * Revision 1.19  2006/04/09 13:07:51  alfred
 * - 1.42 Bld46
 *
 * Revision 1.18  2006/04/04 16:24:23  alfred
 * - 1.42Bld45
 *
 * Revision 1.17  2006/03/18 11:25:55  alfred
 * - Bld44
 *
 * Revision 1.16  2005/12/17 16:36:45  alfred
 * - 1.42 Bld43
 *
 * Revision 1.15  2005/11/04 21:09:08  alfred
 * - 1.42 Bld42
 *
 * Revision 1.14  2005/10/02 12:17:40  alfred
 * - Bld41
 *
 * Revision 1.13  2005/09/17 19:23:16  alfred
 * - Bld 40
 *
 * Revision 1.12  2005/09/12 19:30:26  alfred
 * - Bld39
 *
 * Revision 1.11  2005/09/08 16:13:40  alfred
 * - Bld38
 *
 * Revision 1.10  2005/08/07 10:34:08  alfred
 * - 1.42 Bld37
 *
 * Revision 1.9  2005/08/06 13:37:39  alfred
 * - Build 0036
 *
 * Revision 1.8  2005/03/21 19:28:11  alfred
 * - Bld35
 *
 * Revision 1.7  2004/11/20 17:36:26  alfred
 * - Bld34
 *
 * Revision 1.6  2004/09/26 10:08:35  alfred
 * - Bld33
 *
 * Revision 1.5  2004/05/29 14:39:35  alfred
 * - Bld32 changelog
 *
 * Revision 1.4  2004/03/31 18:25:20  alfred
 * - Bld31
 *
 * Revision 1.3  2003/12/07 13:59:50  alfred
 * - fixed DebVersion
 *
 * Revision 1.2  2003/12/07 13:51:51  alfred
 * - 1.42 Bld 30
 *
 * Revision 1.1  2003/11/06 02:40:28  alfred
 * - recreated
 *
 * Revision 1.12  2003/08/17 14:06:59  alfred
 * - build 29
 *
 * Revision 1.11  2003/06/08 20:17:43  alfred
 * - stuff for Debian archive
 *
 * Revision 1.10  2003/05/25 13:01:12  alfred
 * - Build 28
 *
 * Revision 1.9  2003/03/29 20:00:54  alfred
 * - Build 27
 *
 * Revision 1.8  2003/02/26 19:27:50  alfred
 * - Build 26
 *
 * Revision 1.7  2003/02/02 13:32:24  alfred
 * - Build 25
 *
 * Revision 1.6  2002/11/23 14:48:02  alfred
 * . Build 24
 *
 * Revision 1.5  2002/10/11 08:34:54  alfred
 * - 1.42 Bld 23
 *
 * Revision 1.4  2002/10/09 17:55:07  alfred
 * - 1.42 Bld 22
 *
 * Revision 1.3  2002/05/25 20:39:23  alfred
 * - 1.42 Bld 21
 *
 * Revision 1.2  2002/03/10 10:50:48  alfred
 * - build 20
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>

char *Version="1.42 Beta [Bld 77]";
char *DebVersion = "1.42bld77-1";
LongInt VerNo=0x142f;

char *InfoMessCopyright="(C) 1992,2010 Alfred Arnold";

LongInt Magic=0x12372c45;

void version_init(void)
{
  unsigned int z;
  char *CMess = InfoMessCopyright;
  LongWord XORVal;

  for (z = 0; z < strlen(CMess); z++)
  {
    XORVal = CMess[z];
    XORVal = XORVal << (((z + 1) % 4) * 8);
    Magic = Magic ^ XORVal;
  }
}
