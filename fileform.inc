CONST
   FileMagic=$1489;

   FileHeaderEnd=0;
   FileHeaderStartAdr=$80;
   FileHeaderDataRec=$81;

   SegNone=0;
   SegCode=1;
   SegData=2;
   SegIData=3;
   SegXData=4;
   SegYData=5;
   SegBData=6;
   SegIO=7;
   SegReg=8;

   PCMax=SegReg;

