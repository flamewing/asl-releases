#include "stdinc.h"

#include "chardefs.h"

typedef struct
         {
          int isochar;
          char *syschar;
         } chartrans;

static chartrans specchars[]=
         {{0344,CH_ae},
          {0366,CH_oe},
          {0374,CH_ue},
          {0304,CH_Ae},
          {0326,CH_Oe},
          {0334,CH_Ue},
          {0337,CH_sz},
          {0262,CH_e2},
          {0265,CH_mu},
          {0340,CH_agrave},
          {0000,""}};

#define TMPNAME "tempfile"

	void doexec(char *cmdline)
{
   int res=system(cmdline);
   if (res!=0)
    {
     fprintf(stderr,"command \"%s\" failed\n",cmdline); exit(2);
    }
}

	int main(int argc, char **argv)
{
   FILE *src,*dest;
   int ch;
   unsigned char cmdline[1024];
   long charcnt,metacnt,crcnt;
   chartrans *z2;

   if (argc<2)
    {
     fprintf(stderr,"usage: %s <file> [destfile]\n",argv[0]);
     exit(1);
    }

#ifdef _WIN32
   {
     char *p;
     int z;

     for (z = 1; z < argc; z++)
     {
       argv[z] = strdup(argv[z]);
       for (p = argv[z]; *p; p++)
        if (*p == '\\')
          *p = '/';      
     }
   }
#endif

   src=fopen(argv[1],OPENRDMODE);
   if (src==NULL)
    {
     fprintf(stderr,"error opening %s for reading\n",argv[1]); exit(2);
    }
   dest=fopen((argc==2)?TMPNAME:argv[2],OPENWRMODE);
   if (dest==NULL)
    {
     fprintf(stderr,"error opening %s for writing\n",TMPNAME); exit(2);
    }
   charcnt=metacnt=crcnt=0;
   while (!feof(src))
    {
     ch=fgetc(src); charcnt++;
     switch (ch)
      {
       case EOF:
        break;
       case 10:
        fputc(13,dest); fputc(10,dest); crcnt++; break;
       default:
        for (z2=specchars; z2->isochar!=0000; z2++)
         if (ch==z2->isochar)
          { fputs(z2->syschar,dest); metacnt++; break; }
        if (z2->isochar==0000) fputc(ch,dest);
      }
    }
   fclose(src); fclose(dest);
   if (argc==2)
    BEGIN
#if defined (__MSDOS__) || defined(__EMX__)
     sprintf(cmdline,"copy %s %s",TMPNAME,argv[1]);
#else
     sprintf(cmdline,"cp %s %s",TMPNAME,argv[1]);
#endif
     doexec(cmdline);
     unlink(TMPNAME);
    END
   printf("%s: %ld char(s), %ld cr(s) added, %ld meta char(s) reconverted\n",
          argv[1],charcnt,crcnt,metacnt);

   return 0;
}
