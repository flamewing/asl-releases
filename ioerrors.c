/* ioerrors.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Abliefern der I/O-Fehlermeldungen                                         */
/*                                                                           */
/* Historie: 11.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <errno.h>
#include "ioerrors.rsc"
#include "ioerrors.h"

typedef struct
         {
          int Code;
          char *Msg;
         } ErrorDef;

static ErrorDef ErrorDefs[]={
#ifdef EPERM
     {EPERM       , IoErr_EPERM},
#endif
#ifdef ENOENT
     {ENOENT      , IoErr_ENOENT},
#endif
#ifdef ESRCH
     {ESRCH       , IoErr_ESRCH},
#endif
#ifdef EINTR
     {EINTR       , IoErr_EINTR},
#endif
#ifdef EIO
     {EIO         , IoErr_EIO},
#endif
#ifdef ENXIO
     {ENXIO       , IoErr_ENXIO},
#endif
#ifdef E2BIG
     {E2BIG       , IoErr_E2BIG},
#endif
#ifdef ENOEXEC
     {ENOEXEC     , IoErr_ENOEXEC},
#endif
#ifdef EBADF
     {EBADF       , IoErr_EBADF},
#endif
#ifdef ECHILD
     {ECHILD      , IoErr_ECHILD},
#endif
#ifdef EDEADLK
     {EDEADLK     , IoErr_EDEADLK},
#endif
#ifdef ENOMEM
     {ENOMEM      , IoErr_ENOMEM},
#endif
#ifdef EACCES
     {EACCES      , IoErr_EACCES},
#endif
#ifdef EFAULT
     {EFAULT      , IoErr_EFAULT},
#endif
#ifdef ENOTBLK
     {ENOTBLK     , IoErr_ENOTBLK},
#endif
#ifdef EBUSY
     {EBUSY       , IoErr_EBUSY},
#endif
#ifdef EEXIST
     {EEXIST      , IoErr_EEXIST},
#endif
#ifdef EXDEV
     {EXDEV       , IoErr_EXDEV},
#endif
#ifdef ENODEV
     {ENODEV      , IoErr_ENODEV},
#endif
#ifdef ENOTDIR
     {ENOTDIR     , IoErr_ENOTDIR},
#endif
#ifdef EISDIR
     {EISDIR      , IoErr_EISDIR},
#endif
#ifdef EINVAL
     {EINVAL      , IoErr_EINVAL},
#endif
#ifdef ENFILE
     {ENFILE      , IoErr_ENFILE},
#endif
#ifdef EMFILE
     {EMFILE      , IoErr_EMFILE},
#endif
#ifdef ENOTTY
     {ENOTTY      , IoErr_ENOTTY},
#endif
#ifdef ETXTBSY
     {ETXTBSY     , IoErr_ETXTBSY},
#endif
#ifdef EFBIG
     {EFBIG       , IoErr_EFBIG},
#endif
#ifdef ENOSPC
     {ENOSPC      , IoErr_ENOSPC},
#endif
#ifdef ESPIPE
     {ESPIPE      , IoErr_ESPIPE},
#endif
#ifdef EROFS
     {EROFS       , IoErr_EROFS},
#endif
#ifdef EMLINK
     {EMLINK      , IoErr_EMLINK},
#endif
#ifdef EPIPE
     {EPIPE       , IoErr_EPIPE},
#endif
#ifdef EDOM
     {EDOM        , IoErr_EDOM},
#endif
#ifdef ERANGE
     {ERANGE      , IoErr_ERANGE},
#endif
#ifdef ENAMETOOLONG
     {ENAMETOOLONG, IoErr_ENAMETOOLONG},
#endif
#ifdef ENOLCK
     {ENOLCK      , IoErr_ENOLCK},
#endif
#ifdef ENOSYS
     {ENOSYS      , IoErr_ENOSYS},
#endif
#ifdef ENOTEMPTY
     {ENOTEMPTY   , IoErr_ENOTEMPTY},
#endif
#ifdef ELOOP
     {ELOOP       , IoErr_ELOOP},
#endif
#ifdef EWOULDBLOCK
     {EWOULDBLOCK , IoErr_EWOULDBLOCK},
#endif
#ifdef ENOMSG
     {ENOMSG      , IoErr_ENOMSG},
#endif
#ifdef EIDRM
     {EIDRM       , IoErr_EIDRM},
#endif
#ifdef ECHRNG
     {ECHRNG      , IoErr_ECHRNG},
#endif
#ifdef EL2NSYNC
     {EL2NSYNC    , IoErr_EL2NSYNC},
#endif
#ifdef EL3HLT
     {EL3HLT      , IoErr_EL3HLT},
#endif
#ifdef EL3RST
     {EL3RST      , IoErr_EL3RST},
#endif
#ifdef ELNRNG
     {ELNRNG      , IoErr_ELNRNG},
#endif
#ifdef EUNATCH
     {EUNATCH     , IoErr_EUNATCH},
#endif
#ifdef ENOCSI
     {ENOCSI      , IoErr_ENOCSI},
#endif
     {-1,NULL}};
       
	char *GetErrorMsg(int number)
BEGIN
   static String hs;
   ErrorDef *z;

   for (z=ErrorDefs; z->Msg!=Nil; z++)
    if (number==z->Code) break;

   if (z->Msg!=Nil) return z->Msg;
   else
    BEGIN
     sprintf(hs,"%s%d",IoErrUnknown,number); return hs;
    END
END
