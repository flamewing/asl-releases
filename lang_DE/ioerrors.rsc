/* lang_DE/ioerrors.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Definition Fehlermeldungs-Strings                                         */
/*                                                                           */
/* Historie:  6. 9.1996 Grundsteinlegung                                     */
/*           19. 9.1996 ...EACCESS                                           */
/*                                                                           */
/*****************************************************************************/

#define IoErr_EPERM             "kein Eigent"CH_ue"mer"
#define IoErr_ENOENT            "Datei oder Verzeichnis nicht gefunden"
#define IoErr_ESRCH             "Proze"CH_sz" existiert nicht"
#define IoErr_EINTR             "unterbrochener Systemaufruf"
#define IoErr_EIO               "E/A-Fehler"
#define IoErr_ENXIO             "Ger"CH_ae"t existiert nicht"
#define IoErr_E2BIG             "Parameterliste zu lang"
#define IoErr_ENOEXEC           "Programm-Dateiformatfehler"
#define IoErr_EBADF             "ung"CH_ue"ltiger Dateihandle"
#define IoErr_ECHILD            "Tochterproze"CH_sz" existiert nicht"
#define IoErr_EDEADLK           "Operation w"CH_ue"rde  Deadlock verursachen"
#define IoErr_ENOMEM            "Speicher"CH_ue"berlauf"
#define IoErr_EACCES            "Zugriff verweigert"
#define IoErr_EFAULT            "Adre"CH_sz"fehler"
#define IoErr_ENOTBLK           "Blockger"CH_ae"t erforderlich"
#define IoErr_EBUSY             "Ger"CH_ae"t blockiert"
#define IoErr_EEXIST            "Datei existiert bereits"
#define IoErr_EXDEV             "Link "CH_ue"ber verschiedene Ger"CH_ae"te"
#define IoErr_ENODEV            "Ger"CH_ae"t nicht vorhanden"
#define IoErr_ENOTDIR           "kein Verzeichnis"
#define IoErr_EISDIR            "Verzeichnis"
#define IoErr_EINVAL            "ung"CH_ue"ltiges Argument"
#define IoErr_ENFILE            "Dateitabellen"CH_ue"berlauf"
#define IoErr_EMFILE            "zu viele offene Dateien"
#define IoErr_ENOTTY            "kein Fernschreiber"
#define IoErr_ETXTBSY           "Code-Datei blockiert"
#define IoErr_EFBIG             "Datei zu gro"CH_sz
#define IoErr_ENOSPC            "Dateisystem voll"
#define IoErr_ESPIPE            "Ung"CH_ue"ltige Positionierung"
#define IoErr_EROFS             "Dateisystem nur lesbar"
#define IoErr_EMLINK            "zu viele Links"
#define IoErr_EPIPE             "geplatzter Schlauch ;-)"
#define IoErr_EDOM              "Funktionsargument au"CH_sz"erhalb Definitionsbereich"
#define IoErr_ERANGE            "Funktionsergebnis au"CH_sz"erhalb  Wertebereich"
#define IoErr_ENAMETOOLONG      "Dateiname zu lang"
#define IoErr_ENOLCK            "Datensatzverieglung nicht m"CH_oe"glich"
#define IoErr_ENOSYS            "Funktion nicht implementiert"
#define IoErr_ENOTEMPTY         "Verzeichnis nicht leer"
#define IoErr_ELOOP             "zu viele symbolische Links"
#define IoErr_EWOULDBLOCK       "Operation w"CH_ue"rde blockieren"
#define IoErr_ENOMSG            "keine Nachricht gew"CH_ue"nschten Typs verf"CH_ue"gbar"
#define IoErr_EIDRM             "Kennung entfernt"
#define IoErr_ECHRNG            "Kanalnummer au"CH_sz"erhalb Bereich"
#define IoErr_EL2NSYNC          "Ebene 2 nicht synchronisiert"
#define IoErr_EL3HLT            "Ebene 3 angehalten"
#define IoErr_EL3RST            "Ebene 3 zur"CH_ue"ckgesetzt"
#define IoErr_ELNRNG            "Link-Nummer au"CH_sz"erhalb Bereich"
#define IoErr_EUNATCH           "Protokolltreiber nicht angebunden"
#define IoErr_ENOCSI            "keine CSI-Struktur verf"CH_ue"gbar"

#define IoErrUnknown            "unbekannter Fehler Nr."


