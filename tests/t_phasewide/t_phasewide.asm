 cpu 68000
 padding off
 phase -65536
 if * <> -65536
  fatal "negative phase was truncated"
 endif
 ds.b 1
 if * <> -65535
  fatal "negative phase progression was truncated"
 endif
 dephase
 phase $ffff0000
 if * <> -65536
  fatal "implicit signed RAM phase was lost"
 endif
 dephase
 cpu z380
 extmode on
 phase 2147483648
 if $ <> 2147483648
  fatal "positive phase was sign-extended"
 endif
 ds 1
 if $ <> 2147483649
  fatal "positive phase progression was sign-extended"
 endif
 dephase
 db 170
