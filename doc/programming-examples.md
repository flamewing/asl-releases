# Programming Examples

## 16-bit instructions via macros

An 8-bit processor can combine two registers into a virtual 16-bit accumulator. Macros operating on it should support the same addressing modes as the hardware: immediate, direct (the first 256 bytes), extended and indexed. On Motorola 6800, accumulators A and B can form this pair.

Indexed addressing has two arguments. An immediate argument starts with `#`; convert it to a string to inspect and remove that prefix, then evaluate its value. The assembler can choose direct or extended addressing from the address. This subtraction macro handles all four forms:

```asm
subd    macro   ARG1,ARG2
  if      "ARG2" != ""            ; indexed?
   suba    (ARG1)+1,ARG2
   sbcb    ARG1,ARG2
  elseif                          ; not indexed?
_SARG1   set     "ARG1"           ; convert to string
  if      substr(_SARG1,0,1)='#' ; immediate?
_SARG1    set     substr(_SARG1,1,strlen(_SARG1)-1) ; y->del #
   suba    #lo(VAL(_SARG1))      ; ...and subtract lo/hi bytes
   sbcb    #hi(VAL(_SARG1))
  elseif                         ; no immediate->ext. or direct
   suba    (ARG1)+1              ; and subtract lo/hi bytes
   sbcb    ARG1
  endif
  endif
  endm
```

Uppercase parameter names make the macro work in both case-sensitive and case-insensitive modes. Calls may use any of these addressing forms:

```asm
        subd    $0007                   ; direct
        subd    $1234                   ; absolute
        subd    #$55aa                  ; immediate
        subd    $12,x                   ; indexed
```

A macro can define another macro. A generator macro takes the new instruction name and its low-byte and high-byte operations, avoiding repeated addressing logic:

```asm
def16   macro   NEWINST,LOINST,HIINST
NEWINST macro   ARG1,ARG2
  if      "ARG2" != ""            ; indexed?
   LOINST  (ARG1)+1,ARG2
   HIINST  ARG1,ARG2
  elseif                          ; not indexed?
_SARG1   set     "ARG1"                 ; convert to string
  if      substr(_SARG1,0,1)='#' ; immediate?
_SARG1    set     substr(_SARG1,1,strlen(_SARG1)-1) ; y->del #
   LOINST  #lo(VAL(_SARG1))      ; ...and subtract lo/hi bytes
   HIINST  #hi(VAL(_SARG1))
  elseif                         ; no immediate->ext. or direct
   LOINST  (ARG1)+1              ; ...and subtract lo/hi bytes
   HIINST  ARG1
  endif
  endif
  endm
  endm
```

The remaining instruction definitions are one-line calls:

```asm
        def16   addd,adda,adcb
        def16   subd,suba,sbcb
        def16   andd,anda,andb
        def16   ord,ora,orb
        def16   eord,eora,eorb
```

