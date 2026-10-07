 cpu 8086
 page 0
 dq ~0h,~00h,~000h,~0ah,~0fh,~05h
zero equ 0h
comp equ ~zero
 dq comp,~comp,~(~0h)
