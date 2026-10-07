 page 0
 cpu 34010
 .field 0,1
 .field 1,1
 .field -1,1
 .field -1,1
 expect 1315
 .field -2,1
 endexpect
 expect 1320
 .field 2,1
 endexpect
 .field 0,8
 .field 255,8
 .field -1,8
 .field -128,8
 expect 1315
 .field -129,8
 endexpect
 expect 1320
 .field 256,8
 endexpect
 .field 0,16
 .field 65535,16
 .field -1,16
 .field -32768,16
 expect 1315
 .field -32769,16
 endexpect
 expect 1320
 .field 65536,16
 endexpect
 .field 0,31
 .field 2147483647,31
 .field -1,31
 .field -1073741824,31
 expect 1315
 .field -1073741825,31
 endexpect
 expect 1320
 .field 2147483648,31
 endexpect
 .field 0,32
 .field 4294967295,32
 .field -1,32
 .field -2147483648,32
 expect 1315
 .field 0,0
 endexpect
 expect 1320
 .field 0,33
 endexpect
 .field later34010_1,1
 .field later34010_8,8
 .field later34010_32,32
 .field 0,7
 cpu 34020
 .field 0,1
 .field 1,1
 .field -1,1
 .field -1,1
 expect 1315
 .field -2,1
 endexpect
 expect 1320
 .field 2,1
 endexpect
 .field 0,8
 .field 255,8
 .field -1,8
 .field -128,8
 expect 1315
 .field -129,8
 endexpect
 expect 1320
 .field 256,8
 endexpect
 .field 0,16
 .field 65535,16
 .field -1,16
 .field -32768,16
 expect 1315
 .field -32769,16
 endexpect
 expect 1320
 .field 65536,16
 endexpect
 .field 0,31
 .field 2147483647,31
 .field -1,31
 .field -1073741824,31
 expect 1315
 .field -1073741825,31
 endexpect
 expect 1320
 .field 2147483648,31
 endexpect
 .field 0,32
 .field 4294967295,32
 .field -1,32
 .field -2147483648,32
 expect 1315
 .field 0,0
 endexpect
 expect 1320
 .field 0,33
 endexpect
 .field later34020_1,1
 .field later34020_8,8
 .field later34020_32,32
 .field 0,7
later34010_1 equ -1
later34010_8 equ -128
later34010_32 equ -1
later34020_1 equ -1
later34020_8 equ -128
later34020_32 equ -1
