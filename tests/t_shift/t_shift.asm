          cpu z80

pushlist  macro reg
          if      "REG"<>""
           push    reg
           shift
           pushlist ALLARGS
          endif
          endm

          pushlist af,bc,de,hl,ix,iy

