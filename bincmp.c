#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
   FILE *f1,*f2;
   unsigned char c1,c2;
   long pos=0;
   
   if (argc!=3)
    {
     fprintf(stderr,"calling convention: %s <file1> <file2>\n",argv[0]);
     return 1;
    }

   f1=fopen(argv[1],"rb");
   if (f1==NULL)
    {
     perror(argv[1]); return 2;
    }
   f2=fopen(argv[2],"rb");
   if (f2==NULL)
    {
     perror(argv[2]); return 2;
    }

   while ((!feof(f1))&&(!feof(f2)))
    {
     fread(&c1,1,1,f1); fread(&c2,1,1,f2);
     if (c1!=c2)
      {
       fprintf(stderr,"compare error at position %d\n",pos);
       fclose(f1); fclose(f2); return 3;
      }
     pos++;
    }

   if (feof(f1)!=feof(f2))
    {
     fprintf(stderr,"files have different sizes\n");
     fclose(f1); fclose(f2); return 4;
    }

   fclose(f1); fclose(f2); return 0;
}
