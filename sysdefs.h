/*---------------------------------------------------------------------------*/
/* unify 68K platforms */

#ifdef __mc68020
#ifndef __m68k
#define __m68k
#endif
#endif

/*---------------------------------------------------------------------------*/
/* just a hack to allow distinguishing SunOS from Solaris on Sparcs... */

#ifdef __sparc
#ifndef __SVR4
#define __sunos__
#else
#define __solaris__
#endif
#endif

/*---------------------------------------------------------------------------*/
/* similar on Sun 3's... */

#ifdef __m68k
#ifndef __NetBSD__
#define __sunos__
#endif
#endif

/*===========================================================================*/
/* 68K platforms */

#ifdef __mc68020
#ifndef __m68k
#define __m68k
#endif
#endif

#ifdef __m68k

/*---------------------------------------------------------------------------*/
/* SUN/3 with SunOS 4.x: 

   see my SunOS quarrels in the Sparc section... */

#ifdef __sunos__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#define BRAINDEAD_SYSTEM_WITHOUT_NLS
#endif

/*---------------------------------------------------------------------------*/
/* SUN/3 with NetBSD 1.x: 

   quite a normal 32-Bit-UNIX system */

#ifdef __NetBSD__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* Linux/68K: 

   quite a normal 32-Bit-UNIX system */

#ifdef __linux__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#define BRAINDEAD_SYSTEM_WITHOUT_NLS
#endif

#endif /* __m68k */

/*===========================================================================*/
/* SPARC platforms */

#ifdef __sparc

/*---------------------------------------------------------------------------*/
/* SUN Sparc with SunOS 4.1.x: 

   don't try cc, use gcc, it's hopeless without an ANSI-compliant compiler... 
   SunOS does have NLS support, but it does not have D_FMT and T_FMT
   I should change this ... 
   Though the manual pages claim that memmove and atexit exist, I could not
   find them in any library :-(  Fortunately, bcopy claims to be safe for
   overlapping arrays, we just have to reverse source and destination pointers.
   The sources themselves contain a switch to use on_exit instead of atexit
   (it uses a different callback scheme, so we cannot just make a #define here... 
   To get rid of most of the messages about missing prototypes, add 
   -D__USE_FIXED_PROTOTYPES__ to your compiler flags! 
   Apart from these few points, one could claim SunOS to be quite a normal
   32-bit-UNIX... */

#ifdef __sunos__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#define fpos_t long
extern void bcopy(const void *b1, const void *b2, const int length);
#define memmove(s1,s2,len) bcopy(s2,s1,len)
#define BRAINDEAD_SYSTEM_WITHOUT_NLS
#endif

/*---------------------------------------------------------------------------*/
/* SUN Sparc with Solaris 2.x: 

   quite a normal 32-Bit-UNIX system */

#ifdef __solaris__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* __sparc */

/*===========================================================================*/
/* Mips platforms */

#ifdef __mips

/*---------------------------------------------------------------------------*/
/* R3000 with Ultrix 4.3: 

   nl_langinfo prototype is there, but no function in library ?! 
   use long long only if you have gcc, c89 doesn't like them ! 
   cc isn't worth trying, believe me! */

#ifdef __ultrix
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
#ifdef __GNUC__
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#else
#define NOLONGLONG
#endif
#define BRAINDEAD_SYSTEM_WITHOUT_NLS
#endif

/*---------------------------------------------------------------------------*/
/* R2000/3000 with NetBSD 1.2: 

   quite a normal 32-Bit-UNIX system */

#ifdef __NetBSD__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* R3000/4x00 with Irix 5.x: 

  quite a normal 32-Bit-UNIX system
  seems also to work with 6.2... */

#ifdef __sgi
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* __mips */ 

/*===========================================================================*/
/* HP-PA platforms */

#ifdef __hppa

/*---------------------------------------------------------------------------*/
/* HP-PA 1.x with HP-UX: */

#ifdef __hpux
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* __hppa */ 

/*===========================================================================*/
/* POWER platforms */

#ifdef _POWER

/*---------------------------------------------------------------------------*/
/* POWER with AIX 4.1: */

#ifdef _AIX
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* _POWER */ 

/*===========================================================================*/
/* DEC Alpha platforms */

#ifdef __alpha 

/*---------------------------------------------------------------------------*/
/* DEC Alpha with Digital UNIX and DEC C / GCC:
   
   Alpha is a 64 bit machine, so we do not need to use extra longs
   OSF has full NLS support */

#ifdef __osf__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64
#endif

/*---------------------------------------------------------------------------*/
/* DEC Alpha with Linux and GCC:
   
   see OSF... 
   NLS still missing... */

#ifdef __linux__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long Integ64;
typedef unsigned long Card64;
#define HAS64
#define BRAINDEAD_SYSTEM_WITHOUT_NLS
#endif

#endif /* __alpha */

/*===========================================================================*/
/* Intel i386 platforms */

#ifdef __i386 

/*---------------------------------------------------------------------------*/
/* Intel i386 with Linux and GCC:
   
   principally, a normal 32-bit *NIX */

#ifdef __linux__
#define DEFSMADE
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ16;
typedef unsigned short Card16;
#define HAS16
typedef signed int Integ32;
typedef unsigned int Card32;
typedef signed long long Integ64;
typedef unsigned long long Card64;
#define HAS64
#endif

#endif /* __i386 */

/*===========================================================================*/
/* Misc... */

/*---------------------------------------------------------------------------*/
/* Just for curiosity, it won't work without 16 bit int's... */

#ifdef _CRAYMPP
typedef signed char Integ8;
typedef unsigned char Card8;
typedef signed short Integ32;
typedef unsigned short Card32;
typedef signed int Integ64;
typedef unsigned int Card64;
#define HAS64
#endif

/*===========================================================================*/

#ifndef DEFSMADE
#error "your platform so far is not included in AS's header files!"
#error "please edit sysdefs.h!"
#endif

