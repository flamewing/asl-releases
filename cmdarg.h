#ifndef _CMDARG_H
#define _CMDARG_H
/* cmdarg.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verarbeitung Kommandozeilenparameter                                      */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*            2001-10-20: option string is pointer instead of array          */
/*                                                                           */
/*****************************************************************************/

typedef enum {CMDErr, CMDFile, CMDOK, CMDArg} CMDResult;

typedef CMDResult (*CMDCallback)(
#ifdef __PROTOS__
Boolean NegFlag, const char *Arg
#endif
);

typedef void (*CMDErrCallback)
(
#ifdef __PROTOS__
Boolean InEnv, char *Arg
#endif
);

typedef struct
{
  char *Ident; 
  CMDCallback Callback;
} CMDRec;
         
#define MAXPARAM 256
typedef Boolean CMDProcessed[MAXPARAM + 1];

extern LongInt ParamCount;
extern char **ParamStr;
extern StringList FileArgList;

extern Boolean ProcessedEmpty(CMDProcessed Processed);

extern void ProcessCMD(CMDRec *Def, Integer Cnt, CMDProcessed Unprocessed,
                       char *EnvName, CMDErrCallback ErrProc);

extern const char *GetEXEName(void);

extern void cmdarg_init(char *ProgPath);

#endif /* _CMDARG_H */
