#include <stdio.h>

#define ConstBUF32 65.12
#define ConstFL900 33.52
#define ConstMIC51 359.52

int main(int argc, char **argv)
{
  double v1,v2,v3,res;

  setbuf(stdout,0);
  printf("BUF32="); scanf("%lf",&v1);
  printf("FL900="); scanf("%lf",&v2);
  printf("MIC51="); scanf("%lf",&v3);
  res=((ConstBUF32/v1)+(ConstFL900/v2)+(ConstMIC51/v3))/3;
  printf("Res=%lf\n",res);
  printf("Clk=="); scanf("%lf",&v1);
  printf("Rel_Res==%lf\n",res/v1);
  return 0;
}