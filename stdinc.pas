{$IFDEF MSDOS}
 {$a+,b-,e+,f+,g-,i+,n+,o+,r-,s-,v-,x-}
 {$DEFINE SPEEDUP}
{$ENDIF}

{$IFDEF DPMI}
 {$a+,b-,e+,f-,g+,i+,n+,o-,r-,s-,v-,x-}
 {$DEFINE DPMISERVER}
 {$DEFINE SEGATTRS}
 {$DEFINE SPEEDUP}
{$ENDIF}

{$IFDEF WINDOWS}
 {$a+,b-,f-,g+,i+,n+,r-,s-,v-,w-,x+}
 {$DEFINE DPMISERVER}
 {$DEFINE SEGATTRS}
 {$DEFINE SPEEDUP}
{$ENDIF}

{$IFDEF OS2}
 {$a+,b-,e+,f-,g+,i+,n+,o-,r-,s-,v-,x-}
 {$DEFINE SEGATTRS}
 {$IFNDEF VIRTUALPASCAL}
  {$DEFINE SPEEDUP}
 {$ENDIF}
{$ENDIF}
