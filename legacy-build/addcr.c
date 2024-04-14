#include <stdio.h>

int main(int argc, char **argv)
{
  int ch;
  
  (void)argc; (void)argv;
  while (!feof(stdin))
   {
    ch=fgetc(stdin);
    if (ch==10) fputc(13,stdout);
    if (ch!=EOF) fputc(ch,stdout);
   }
  return 0;
}