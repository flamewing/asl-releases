/* decodecmd.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verarbeitung Kommandozeilenparameter                                      */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef enum {CMDErr,CMDFile,CMDOK,CMDArg} CMDResult;
typedef CMDResult (*CMDCallback)(Boolean NegFlag, char *Arg);
typedef void (*CMDErrCallback)(Boolean InEnv, char *Arg);

typedef struct
         {
          char Ident[11]; 
          CMDCallback Callback;
         } CMDRec;
         
#define MAXPARAM 256
typedef Boolean CMDProcessed[MAXPARAM+1];         

extern LongInt ParamCount;
extern char **ParamStr;


extern Boolean ProcessedEmpty(CMDProcessed Processed);

extern void ProcessCMD(CMDRec *Def, Integer Cnt, CMDProcessed Unprocessed,
                       char *EnvName, CMDErrCallback ErrProc);

extern char *GetEXEName(void);

extern void decodecmd_init(void);
