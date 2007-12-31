/* findhyphen.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Zerlegung von Worten in Silben gemaess dem TeX-Algorithmus                */
/*                                                                           */
/* Historie: 17.2.1998 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#undef DEBUG

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"

#ifdef DEBUG
#include "ushyph.h"
#endif

/*****************************************************************************/

#define LCNT (26+1+4)
#define SEPCNT 10

typedef struct _THyphenNode
         {
          Byte sepcnts[SEPCNT];
          struct _THyphenNode *Daughters[LCNT];
         } THyphenNode,*PHyphenNode;

typedef struct _THyphenException
         {
          struct _THyphenException *next;
          char *word;
          int poscnt,*posis;
         } THyphenException,*PHyphenException;

#define INTCHR_AE '\344'
#define INTCHR_OE '\366'
#define INTCHR_UE '\374'
#define INTCHR_SZ '\337'

/*****************************************************************************/

static PHyphenNode HyphenRoot=Nil;
static PHyphenException FirstException=Nil;

/*****************************************************************************/

#if 0
char b[10];

	static void PrintNode(PHyphenNode Node, int Level)
BEGIN
   int z;

   for (z=1; z<Level; z++) putchar(' ');
   for (z=1; z<=Level; z++) putchar(b[z]);
   for (z=0; z<SEPCNT; z++) if (Node->sepcnts[z]>0) break;
   if (z<SEPCNT) putchar('!'); printf(" %p",Node);
   puts("");
   for (z=0; z<LCNT; z++)
    if (Node->Daughters[z]!=Nil)
     BEGIN
      b[Level+1]=z+'a'-1; PrintNode(Node->Daughters[z],Level+1);
     END
END
#endif

	static int GetIndex(char ch)
BEGIN
   if ((mytolower(ch)>='a'-1) AND (mytolower(ch)<='z')) return (mytolower(ch)-('a'-1));
   else if (ch=='.') return 0;
#ifndef CHARSET_ASCII7
   else if ((ch==*CH_ae) OR (ch==*CH_Ae) OR (ch==INTCHR_AE)) return 27;
   else if ((ch==*CH_oe) OR (ch==*CH_Oe) OR (ch==INTCHR_OE)) return 28;
   else if ((ch==*CH_ue) OR (ch==*CH_Ue) OR (ch==INTCHR_UE)) return 29;
   else if ((ch==*CH_sz) OR (ch==INTCHR_SZ)) return 30;
#endif
   else { printf("unallowed character %d\n",ch); return -1; }
END

	static void InitHyphenNode(PHyphenNode Node)
BEGIN
   int z;

   for (z=0; z<LCNT; Node->Daughters[z++]=Nil);
   for (z=0; z<SEPCNT; Node->sepcnts[z++]=0);
END

	void BuildTree(char **Patterns)
BEGIN
   char **run,ch,*pos,sing[500],*rrun;
   Byte RunCnts[SEPCNT];
   int z,l,rc,index;
   PHyphenNode Lauf;

   HyphenRoot=(PHyphenNode) malloc(sizeof(THyphenNode));
   InitHyphenNode(HyphenRoot);

   for (run=Patterns; *run!=NULL; run++)
    BEGIN
     strcpy(sing,*run); rrun=sing;
     do
      BEGIN
       pos=strchr(rrun,' '); if (pos!=Nil) *pos='\0';
       l=strlen(rrun); rc=0; Lauf=HyphenRoot;
       for (z=0; z<SEPCNT; RunCnts[z++]=0);
       for (z=0; z<l; z++)
        BEGIN
         ch=rrun[z];
         if ((ch>='0') AND (ch<='9')) RunCnts[rc]=ch-'0';
         else
          BEGIN
           index=GetIndex(ch);
           if (Lauf->Daughters[index]==Nil)
            BEGIN
             Lauf->Daughters[index]=(PHyphenNode) malloc(sizeof(THyphenNode));
             InitHyphenNode(Lauf->Daughters[index]);
            END
           Lauf=Lauf->Daughters[index]; rc++;
          END
        END
       memcpy(Lauf->sepcnts,RunCnts,sizeof(Byte)*SEPCNT);
       if (pos!=Nil) rrun=pos+1;
      END
     while (pos!=Nil);
    END
END

	void AddException(char *Name)
BEGIN
   char tmp[300],*dest,*src;
   int pos[100];
   PHyphenException New;

   New=(PHyphenException) malloc(sizeof(THyphenException));
   New->next=FirstException;
   New->poscnt=0; dest=tmp;
   for (src=Name; *src!='\0'; src++)
    if (*src=='-') pos[New->poscnt++]=dest-tmp;
    else *(dest++)=*src;
   *dest='\0';
   New->word=strdup(tmp);
   New->posis=(int *) malloc(sizeof(int)*New->poscnt);
   memcpy(New->posis,pos,sizeof(int)*New->poscnt);
   FirstException=New;
END

	void DestroyNode(PHyphenNode Node)
BEGIN
   int z;

   for (z=0; z<LCNT; z++)
    if (Node->Daughters[z]!=Nil) DestroyNode(Node->Daughters[z]);
   free(Node);
END

	void DestroyTree(void)
BEGIN
   PHyphenException Old;

   if (HyphenRoot!=Nil) DestroyNode(HyphenRoot); HyphenRoot=Nil;

   while (FirstException!=Nil)
    BEGIN
     Old=FirstException; FirstException=Old->next;
     free(Old->word); if (Old->poscnt>0) free(Old->posis);
    END
END

	void DoHyphens(char *word, int **posis, int *posicnt)
BEGIN
   char Field[300];
   Byte Res[300];
   int z,z2,z3,l;
   PHyphenNode Lauf;
   PHyphenException Ex;
   
   for (Ex=FirstException; Ex!=Nil; Ex=Ex->next)
    if (strcasecmp(Ex->word,word)==0)
     BEGIN
      *posis=(int *) malloc(sizeof(int)*Ex->poscnt);
      memcpy(*posis,Ex->posis,sizeof(int)*Ex->poscnt);
      *posicnt=Ex->poscnt;
      return;
     END

   l=strlen(word); *posicnt=0;
   *Field='a'-1; 
   for (z=0; z<l; z++) 
    BEGIN
     Field[z+1]=tolower((unsigned int) word[z]);
     if (GetIndex(Field[z+1])<=0) return;
    END
   Field[l+1]='a'-1; l+=2;
   for (z=0; z<=l+1; Res[z++]=0);
 
   if (HyphenRoot==Nil) return;

   for (z=0; z<l; z++)
    BEGIN
     Lauf=HyphenRoot;
     for (z2=z; z2<l; z2++)
      BEGIN
       Lauf=Lauf->Daughters[GetIndex(Field[z2])];
       if (Lauf==Nil) break;
#ifdef DEBUG
       for (z3=0; z3<SEPCNT; z3++) if (Lauf->sepcnts[z3]>0) break;
       if (z3<SEPCNT)
        BEGIN
         printf("Apply pattern ");
         for (z3=z; z3<=z2; putchar(Field[z3++]));
         printf(" at position %d with values",z);
         for (z3=0; z3<SEPCNT; printf(" %d",Lauf->sepcnts[z3++]));
         puts("");
        END
#endif
       for (z3=0; z3<=z2-z+2; z3++)
        if (Lauf->sepcnts[z3]>Res[z+z3]) Res[z+z3]=Lauf->sepcnts[z3];
      END
    END

#ifdef DEBUG
   for (z=0; z<l; z++) printf(" %c",Field[z]); puts("");
   for (z=0; z<=l; z++) printf("%d ",Res[z]); puts("");
   for (z=0; z<l-2; z++) 
    BEGIN
     if ((z>0) AND ((Res[z+1])&1)) putchar('-');
     putchar(Field[z+1]);
    END
   puts("");
#endif

   *posis=(int *) malloc(sizeof(int)*l); *posicnt=0;
   for (z=3; z<l-2; z++) 
    if ((Res[z]&1)==1) (*posis)[(*posicnt)++]=z-1;
   if (*posicnt==0)
    BEGIN
     free(*posis); *posis=Nil;
    END
END

/*****************************************************************************/

#ifdef DEBUG
	int main(int argc, char **argv)
BEGIN
   int z,z2,cnt,*posis,posicnt;

   BuildTree(USHyphens);
   for (z=1; z<argc; z++)
    BEGIN
     DoHyphens(argv[z],&posis,&cnt);
     for (z2=0; z2<cnt; printf("%d ",posis[z2++])); puts("");
     if (posicnt>0) free(posis);
    END
/*   DoHyphens("hyphenation");
   DoHyphens("concatenation");
   DoHyphens("supercalifragilisticexpialidocous");*/
END	
#endif
