/* lang_EN/ioerrors.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Definition Fehlermeldungs-Strings                                         */                    
/* englische Version                                                         */
/*                                                                           */
/* Historie:  6. 9.1996 Grundsteinlegung                                     */                    
/*           19. 9.1996 ...EACCESS                                           */
/*                                                                           */
/*****************************************************************************/

#define IoErr_EPERM             "Not owner"
#define IoErr_ENOENT            "No such file or directory"
#define IoErr_ESRCH             "No such process"
#define IoErr_EINTR             "Interrupted system call"
#define IoErr_EIO               "I/O error"
#define IoErr_ENXIO             "No such device or address"
#define IoErr_E2BIG             "Arg list too long"
#define IoErr_ENOEXEC           "Exec format error"
#define IoErr_EBADF             "Bad file number"
#define IoErr_ECHILD            "No children"
#define IoErr_EDEADLK           "Operation would cause deadlock"
#define IoErr_ENOMEM            "Not enough core"
#define IoErr_EACCES            "Permission denied"
#define IoErr_EFAULT            "Bad address"
#define IoErr_ENOTBLK           "Block device required"
#define IoErr_EBUSY             "Device or resource busy"
#define IoErr_EEXIST            "File exists"
#define IoErr_EXDEV             "Cross-device link"
#define IoErr_ENODEV            "No such device"
#define IoErr_ENOTDIR           "Not a directory"
#define IoErr_EISDIR            "Is a directory"
#define IoErr_EINVAL            "Invalid argument"
#define IoErr_ENFILE            "File table overflow"
#define IoErr_EMFILE            "Too many open files"
#define IoErr_ENOTTY            "Not a typewriter"   
#define IoErr_ETXTBSY           "Text file busy"
#define IoErr_EFBIG             "File too large"   
#define IoErr_ENOSPC            "No space left on device" 
#define IoErr_ESPIPE            "Illegal seek"     
#define IoErr_EROFS             "Read-only file system"
#define IoErr_EMLINK            "Too many links"
#define IoErr_EPIPE             "Broken pipe"
#define IoErr_EDOM              "Math argument out of domain of func"
#define IoErr_ERANGE            "Math result not representable"
#define IoErr_ENAMETOOLONG      "File name too long"
#define IoErr_ENOLCK            "No record locks available"
#define IoErr_ENOSYS            "Function not implemented"
#define IoErr_ENOTEMPTY         "Directory not empty"
#define IoErr_ELOOP             "Too many symbolic links encountered"
#define IoErr_EWOULDBLOCK       "Operation would block"
#define IoErr_ENOMSG            "No message of desired type"
#define IoErr_EIDRM             "Identifier removed"
#define IoErr_ECHRNG            "Channel number out of range"
#define IoErr_EL2NSYNC          "Level 2 not synchronized"
#define IoErr_EL3HLT            "Level 3 halted"
#define IoErr_EL3RST            "Level 3 reset"
#define IoErr_ELNRNG            "Link number out of range"
#define IoErr_EUNATCH           "Protocol driver not attached"
#define IoErr_ENOCSI            "No CSI structure available"

#define IoErrUnknown            "unknown error no."
