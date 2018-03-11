;----------------------------------------------------------------------------
; Fliesskommaroutinen fuer den PcPar68000 - Version mit 68881

;-----------------------------------------------------------------------------
; Definitionen:

CoConst1        equ             $32             ; Offsets im Konstantenrom
CoConstPi       equ             0               ; des 6888x

;-----------------------------------------------------------------------------
; Librarykopf:


S_Float81Lib:   dc.l            S_float81libend-S_float81libstart ; Laenge
S_float81libstart:
                dc.l            -1              ; Speicher fuer Zeiger
		dc.b            "FLOAT",0       ; Name
                ds              0


;-----------------------------------------------------------------------------
; Sprungtabelle:

                bra.l           S_fadd_co68
		bra.l		S_fsub_co68
		bra.l		S_fmul_co68
		bra.l		S_fdiv_co68
		bra.l		S_fsqrt_co68
                bra.l           S_float81libnop
                bra.l           S_float81libnop
                bra.l           S_fcmp_co68
		bra.l		S_fitof_co68
		bra.l		S_fftoi_co68
                bra.l           S_fabs_co68
                bra.l           S_float81libnop
                bra.l           S_float81libnop
                bra.l           S_float81libnop
                bra.l           S_float81libnop
		bra.l           S_fexp_co68
		bra.l		S_fsinh_co68
		bra.l		S_fcosh_co68
		bra.l		S_ftanh_co68
		bra.l		S_fcoth_co68
                bra.l           S_float81libnop
                bra.l           S_float81libnop
                bra.l           S_float81libnop
		bra.l           S_fln_co68
		bra.l		S_flog_co68
		bra.l		S_fasinh_co68
		bra.l		S_facosh_co68
		bra.l		S_fatanh_co68
		bra.l		S_facoth_co68
                bra.l           S_float81libnop
                bra.l           S_float81libnop
		bra.l           S_fsin_co68
		bra.l		S_fcos_co68
		bra.l		S_ftan_co68
		bra.l		S_fcot_co68
                bra.l           S_float81libnop
                bra.l           S_float81libnop
		bra.l           S_float81libnop
                bra.l           S_float81libnop
		bra.l           S_fasin_co68
		bra.l		S_facos_co68
		bra.l		S_fatan_co68
		bra.l		S_facot_co68

S_float81libnop: rts

;----------------------------------------------------------------------------

                fpu             on
S_fadd_co68:
		addq.l          #1,_fadd_cnt.w
                fmove.s         d0,fp0
                fadd.s          d1,fp0
                fmove.s         fp0,d0
		rts


S_fsub_co68:
		addq.l          #1,_fadd_cnt.w
                fmove.s         d0,fp0
                fsub.s          d1,fp0
                fmove.s         fp0,d0
                rts


S_fmul_co68:
		addq.l          #1,_fmul_cnt.w
                fmove.s         d0,fp0
                fsglmul.s       d1,fp0
                fmove.s         fp0,d0
		rts


S_fdiv_co68:
		addq.l          #1,_fdiv_cnt.w
                fmove.s         d0,fp0
                fsgldiv.s       d1,fp0
                fmove.s         fp0,d0
		rts


S_fmul2_co68:
		addq.l          #1,_fmul_cnt.w
                fmove.s         d0,fp0
                fscale.w        d1,fp0
                fmove.s         fp0,d0
		rts


S_fitof_co68:
                fmove.l         d0,fp0
                fmove.s         fp0,d0
		rts


S_fftoi_co68:
                fmove.s         d0,fp0
                fmove.l         fp0,d0
                rts


S_fsqrt_co68:
                addq.l          #1,_fsqrt_cnt.w
                fsqrt.s         d0,fp0
                fmove.s         fp0,d0
		rts


S_fabs_co68:    bclr    #31,d0                  ; ganz einfach...
                rts


S_fcmp_co68:
                fmove.s         d0,fp0
                fcmp.s          d1,fp0
                fbeq.l          S_fcmp_coeq     ; Variante 1:gleich
                fbgt.l          S_fcmp_cogt     ; Variante 2:groeer

                moveq           #1,d0           ; Bedingung "kleiner"
                cmp.w           #2,d0           ; erzeugen
                rts
S_fcmp_cogt:
                moveq           #2,d0           ; Bedingung "groesser"
                cmp.w           #1,d0           ; erzeugen
                rts
S_fcmp_coeq:
                cmp.w           d0,d0           ; Bedingung "gleich"
                rts                             ; erzeugen


S_fexp_co68:
                fetox.s         d0,fp0
                fmove.s         fp0,d0
		rts


S_fsinh_co68:
                fsinh.s         d0,fp0
                fmove.s         fp0,d0
		rts


S_fcosh_co68:
                fcosh.s         d0,fp0
                fmove.s         fp0,d0
		rts


S_ftanh_co68:
                ftanh.s         d0,fp0
                fmove.s         fp0,d0
		rts


S_fcoth_co68:
                fmove.s         d0,fp0
                fsinh.x         fp0,fp1
                fcosh.x         fp0
                fsgldiv.x       fp1,fp0
                fmove.s         fp0,d0
		rts


S_fln_co68:
                flogn.s         d0,fp0
                fmove.s         fp0,d0
		rts


S_flog_co68:
                flog10.s        d0,fp0
                fmove.s         fp0,d0
		rts


S_fasinh_co68:
                fmove.s         d0,fp0
                fmove.x         fp0,fp1
                fmul.x          fp1,fp0
                fmovecr.x       #CoConst1,fp2
                fadd.x          fp2,fp0
                fsqrt.x         fp0
                fadd.x          fp1,fp0
                flogn.x         fp0
                fmove.s         fp0,d0
                rts


S_facosh_co68:
                fmove.s         d0,fp0
                fmove.x         fp0,fp1
                fmul.x          fp1,fp0
                fmovecr.x       #CoConst1,fp2
                fsub.x          fp2,fp0
                fsqrt.x         fp0
                fadd.x          fp1,fp0
                flogn.x         fp0
                fmove.s         fp0,d0
                rts


S_fatanh_co68:
                fatanh.s        d0,fp0
                fmove.s         fp0,d0
                rts


S_facoth_co68:
                fmovecr.x       #CoConst1,fp0
                fdiv.s          d0,fp0
                fatanh.x        fp0
                fmove.s         fp0,d0
                rts


S_fcos_co68:
                fcos.s          d0,fp0
                fmove.x         fp0,d0
                rts


S_fsin_co68:
                fsin.s          d0,fp0
                fmove.s         fp0,d0
                rts


S_ftan_co68:
                ftan.s          d0,fp0
                fmove.s         fp0,d0
                rts


S_fcot_co68:
                fsincos.s       d0,fp0:fp1
                fsgldiv.x       fp1,fp0
                fmove.s         fp0,d0
                rts


S_fatan_co68:
                fatan.s         d0,fp0
                fmove.s         fp0,d0
		rts


S_facot_co68:
                fatan.s         d0,fp1
                fmovecr.x       #CoConstPi,fp0
                fscale.w        #-1,fp0
                fsub.x          fp1,fp0
                fmove.s         fp0,d0
		rts


S_fasin_co68:
                fasin.s         d0,fp0
                fmove.s         fp0,d0
		rts


S_facos_co68:
                facos.s         d0,fp0
                fmove.s         fp0,d0
		rts

S_float81libend:

                fpu             off

