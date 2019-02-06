{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei IOERRORS.RSC - enth„lt I/O- Fehlermeldungen     (englisch)   *}
{* 									    *}
{* Historie : 5.10.1993 Grundsteinlegung                                    *}
{*                      detailliertere Formatfehlermeldungen                *}
{*            6.10.1993 Umstellung auf Englisch begonnen                    *}
{*                                                                          *}
{****************************************************************************}

CONST
   IoErrFileNotFound        = 'file not found';
   IoErrPathNotFound        = 'path not found';
   IoErrTooManyOpenFiles    = 'too many open files';
   IoErrAccessDenied        = 'file access denied';
   IoErrInvHandle           = 'invalid file handle';
   IoErrInvAccMode          = 'invalid access mode';
   IoErrInvDriveLetter      = 'invalid drive letter';
   IoErrCannotRemoveActDir  = 'current directory cannot be deleted';
   IoErrNoRenameAcrossDrives= 'RENAME cannot cross drives';
   IoErrFileEnd             = 'unexpected end of file';
   IoErrDiskFull            = 'disk full';
   IoErrMissingAssign       = 'missingASSIGN';
   IoErrFileNotOpen         = 'file not open';
   IoErrNotOpenForRead      = 'file not open for input';
   IoErrNotOpenForWrite     = 'file not open for output';
   IoErrInvNumFormat        = 'invalid numeric format';
   IoErrWriteProtect        = 'disk is write-protected';
   IoErrUnknownDevice       = 'unknown device';
   IoErrDrvNotReady         = 'drive not ready';
   IoErrUnknownDOSFunc      = 'invalid DOS function';
   IoErrCRCError            = 'CRC error on disk';
   IoErrInvDPB              = 'invalid DPB';
   IoErrPositionErr         = 'seek error';
   IoErrInvSecFormat        = 'invalid sector format';
   IoErrSectorNotFound      = 'sector not found';
   IoErrPaperEnd            = 'paper end';
   IoErrDevReadError        = 'device read error';
   IoErrDevWriteError       = 'device write error';
   IoErrGenFailure          = 'general failure';
   IoErrUnknown             = 'unknown error no.';


