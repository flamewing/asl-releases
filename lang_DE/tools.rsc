/* lang_DE/tools.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Stringdefinitionen, die alle AS-Tools brauchen                            */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Durchsagen */

#define InfoMessHead1            "Aufruf: "


/****************************************************************************/
/* Fehlermeldungen */


#define FormatErr1aMsg           "Das Format der Datei \""
#define FormatErr1bMsg           "\" ist fehlerhaft!"
#define FormatErr2Msg            "Bitte "CH_ue"bersetzen Sie die Quelldatei neu!"

#define FormatInvHeaderMsg       "ung"CH_ue"ltiger Dateikopf"
#define FormatInvRecordHeaderMsg "ung"CH_ue"ltiger Datensatzkopf"
#define FormatInvRecordLenMsg    "fehlerhafte Datensatzl"CH_ae"nge"

#define IOErrAHeaderMsg          "Bei Bearbeitung der Datei \""
#define IOErrBHeaderMsg          "\" ist folgender Fehler aufgetreten:"

#define ErrMsgTerminating        "Das Programm wird beendet!"

#define ErrMsgNullMaskA          "Warnung : Dateimaske "
#define ErrMsgNullMaskB          " pa"CH_sz"t auf keine Datei!"

#define ErrMsgInvEnvParam        "Fehlerhafter Environment-Parameter : "
#define ErrMsgInvParam           "Fehlerhafter Parameter : "

#define ErrMsgTargMissing        "Zieldateiangabe fehlt!"
#define ErrMsgAutoFailed         "automatische Bereichserkennung fehlgeschlagen!"

#define ErrMsgOverlap            "Warnung: "CH_ue"berlappende Belegung!"

#define ErrMsgProgTerm           "Programmabbruch"

/****************************************************************************/

#define Suffix ".p"              /* Suffix Eingabedateien */
