		ifndef	__s12z_adc_inc
__s12z_adc_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZADC.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z ADC                       *
;*                                                                          *
;****************************************************************************

		padding	off

S12ZADC		STRUCT	dots
ADCCTL_0	ds.b	1		; ($00) ADC Control Register 0
ADC_EN		s12zbit	ADCCTL_0,7	;  ADC Enable Bit
ADC_SR		s12zbit	ADCCTL_0,6	;  ADC Soft-Reset
FRZ_MOD		s12zbit	ADCCTL_0,5	;  Freeze Mode Configuration
SWAI		s12zbit	ADCCTL_0,4	;  Wait Mode Configuration
ACC_CFG		s12zfld	ADCCTL_0,2:2	;  ADCFLWCTL Register Access Configuration
STR_SEQA	s12zbit	ADCCTL_0,1	;  Control Of Conversion Result Storage and RSTAR_EIF flag setting at Sequence Abort or Restart Event
MOD_CFG		s12zbit	ADCCTL_0,0	;  (Conversion Flow Control) Mode Configuration
ADCCTL_1	ds.b	1		; ($01) ADC Control Register 1
CSL_BMOD	s12zbit	ADCCTL_1,7	;  CSL Buffer Mode Select Bit
RVL_BMOD	s12zbit	ADCCTL_1,6	;  RVL Buffer Mode Select Bit
SMOD_ACC	s12zbit	ADCCTL_1,5	;  Special Mode Access Control Bit
AUT_RSTA	s12zbit	ADCCTL_1,4	;  Automatic Restart Event after exit from MCU Stop and Wait Mode (SWAI set)
ADCSTS		ds.b	1		; ($02) ADC Status Register
CSL_SEL		s12zbit	ADCSTS,7	;  Command Sequence List Select bit
RVL_SEL		s12zbit	ADCSTS,6	;  Result Value List Select Bit
DBECC_ERR	s12zbit	ADCSTS,5	;  Double Bit ECC Error Flag
READY		s12zbit	ADCSTS,3	;  Ready For Restart Event Flag
ADCTIM		ds.b	1		; ($03) ADC Timing Register
PRS		s12zfld	ADCTIM,7:0	;  ADC Clock Prescale
ADCFMT		ds.b	1		; ($04) ADC Format Register
DJM		s12zbit	ADCFMT,7	;  Result Register Data Justification
SRES		s12zfld	ADCFMT,3:0	;  ADC Resolution Select
ADCFLWCTL	ds.b	1		; ($05) ADC Conversion Flow Control Register
SEQA		s12zbit	ADCFLWCTL,7	;  Conversion Sequence Abort Event
TRIG		s12zbit	ADCFLWCTL,6	;  Conversion Sequence Trigger Bit
RSTA		s12zbit	ADCFLWCTL,5	;  Restart Event (Restart from Top of Command Sequence List)
LDOK		s12zbit	ADCFLWCTL,4	;  Load OK for alternative Command Sequence List
ADCEIE		ds.b	1		; ($06) ADC Error Interrupt Enable Register
IA_EIE		s12zbit	ADCEIE,7	;  Illegal Access Error Interrupt Enable Bit
CMD_EIE		s12zbit	ADCEIE,6	;  Command Value Error Interrupt Enable Bit
EOL_EIE		s12zbit	ADCEIE,5	;  "End Of List" Error Interrupt Enable Bit
TRIG_EIE	s12zbit	ADCEIE,3	;  Conversion Sequence Trigger Error Interrupt Enable Bit
RSTAR_EIE	s12zbit	ADCEIE,2	;  Restart Request Error Interrupt Enable Bit
LDOK_EIE	s12zbit	ADCEIE,1	;  Load OK Error Interrupt Enable Bit
ADCIE		ds.b	1		; ($07) ADC Interrupt Enable Register
SEQAD_IE	s12zbit	ADCIE,7		;  Conversion Sequence Abort Done Interrupt Enable Bit
CONIF_OIE	s12zbit	ADCIE,6		;  ADCCONIF Register Flags Overrun Interrupt Enable
ADCEIF		ds.b	1		; ($08) ADC Error Interrupt Flag Register
IA_EIF		s12zbit	ADCEIF,7	;  Illegal Access Error Interrupt Flag
CMD_EIF		s12zbit	ADCEIF,6	;  Command Value Error Interrupt Flag
EOL_EIF		s12zbit	ADCEIF,5	;  "End Of List" Error Interrupt Flag -
TRIG_EIF	s12zbit	ADCEIF,3	;  Trigger Error Interrupt Flag
RSTAR_EIF	s12zbit	ADCEIF,2	;  Restart Request Error Interrupt Flag
LDOK_EIF	s12zbit	ADCEIF,1	;  Load OK Error Interrupt Flag
ADCIF		ds.b	1		; ($09) ADC Interrupt Flag Register
SEQAD_IF	s12zbit	ADCIF,7		;  Conversion Sequence Abort Done Interrupt Flag
CONIF_OIF	s12zbit	ADCIF,6		;  ADCCONIF Register Flags Overrun Interrupt Flag
ADCCONIE_0	ds.b	1		; ($0a) ADC Conversion Interrupt Enable Register MSB
CON_IE15	s12zbit	ADCCONIE_0,7	;  Conversion Interrupt Enable Bits
CON_IE14	s12zbit	ADCCONIE_0,6
CON_IE13	s12zbit	ADCCONIE_0,5
CON_IE12	s12zbit	ADCCONIE_0,4
CON_IE11	s12zbit	ADCCONIE_0,3
CON_IE10	s12zbit	ADCCONIE_0,2
CON_IE9		s12zbit	ADCCONIE_0,1
CON_IE8		s12zbit	ADCCONIE_0,0
ADCCONIE_1	ds.b	1		; ($0b) ADC Conversion Interrupt Enable Register LSB
CON_IE7		s12zbit	ADCCONIE_1,7
CON_IE6		s12zbit	ADCCONIE_1,6
CON_IE5		s12zbit	ADCCONIE_1,5
CON_IE4		s12zbit	ADCCONIE_1,4
CON_IE3		s12zbit	ADCCONIE_1,3
CON_IE2		s12zbit	ADCCONIE_1,2
CON_IE1		s12zbit	ADCCONIE_1,1
EOL_IE		s12zbit	ADCCONIE_1,0	;  End Of List Interrupt Enable Bit
ADCCONIF_0	ds.b	1		; ($0c) ADC Conversion Interrupt Flag Register MSB
CON_IF15	s12zbit	ADCCONIF_0,7	;  Conversion Interrupt Flags
CON_IF14	s12zbit	ADCCONIF_0,6
CON_IF13	s12zbit	ADCCONIF_0,5
CON_IF12	s12zbit	ADCCONIF_0,4
CON_IF11	s12zbit	ADCCONIF_0,3
CON_IF10	s12zbit	ADCCONIF_0,2
CON_IF9		s12zbit	ADCCONIF_0,1
CON_IF8		s12zbit	ADCCONIF_0,0
ADCCONIF_1	ds.b	1		; ($0d) ADC Conversion Interrupt Flag Register LSB
CON_IF7		s12zbit	ADCCONIF_1,7
CON_IF6		s12zbit	ADCCONIF_1,6
CON_IF5		s12zbit	ADCCONIF_1,5
CON_IF4		s12zbit	ADCCONIF_1,4
CON_IF3		s12zbit	ADCCONIF_1,3
CON_IF2		s12zbit	ADCCONIF_1,2
CON_IF1		s12zbit	ADCCONIF_1,1
EOL_IF		s12zbit	ADCCONIF_1,0	;  End Of List Interrupt Flag
ADCIMDRI_0	ds.b	1		; ($0e) ADC Intermediate Result Information Register MSB
CSL_IMD		s12zbit	ADCIMDRI_0,7	;  Active CSL At Intermediate Event
RVL_IMD		s12zbit	ADCIMDRI_0,6	;  Active RVL At Intermediate Event
ADCIMDRI_1	ds.b	1		; ($0f) ADC Intermediate Result Information Register LSB
RIDX_IMD5	s12zbit	ADCIMDRI_1,5	;  RES_IDX Value At Intermediate Event
RIDX_IMD4	s12zbit	ADCIMDRI_1,4
RIDX_IMD3	s12zbit	ADCIMDRI_1,3
RIDX_IMD2	s12zbit	ADCIMDRI_1,2
RIDX_IMD1	s12zbit	ADCIMDRI_1,1
RIDX_IMD0	s12zbit	ADCIMDRI_1,0
ADCEOLRI	ds.b	1		; ($10) ADC End Of List Result Information Register
CSL_EOL		s12zbit	ADCEOLRI,7	;  Active CSL When "End Of List" Command Type Executed
RVL_EOL		s12zbit	ADCEOLRI,6	;  Active RVL When "End Of List" Command Type Executed
		ds.b	3
ADCCMD_0	ds.b	1		; ($14) ADC Command Register 0
CMD_SEL		s12zfld	ADCCMD_0,2:6	;  Conversion Command Select Bits
INTFLG_SEL	s12zfld	ADCCMD_0,4:0	;  Conversion Interrupt Flag Select Bits
ADCCMD_1	ds.b	1		; ($15) ADC Command Register 1
VRH_SEL		s12zbit	ADCCMD_1,7	;  Reference High Voltage Select Bit
VRL_SEL		s12zbit	ADCCMD_1,6	;  Reference Low Voltage Select Bit
CH_SEL		s12zfld	ADCCMD_1,6:0	;  ADC Input Channel Select Bits
ADCCMD_2	ds.b	1		; ($16) ADC Command Register 2
SMP		s12zfld	ADCCMD_2,5:3	;  Sample Time Select Bits
ADCCMD_3	ds.b	1		; ($17) ADC Command Register 3
		ds.b	4
ADCCIDX		ds.b	1		; ($1c) ADC Command Index Register
CMD_IDX		s12zfld	ADCCIDX,6:0	;  ADC Command Index Bits
ADCCBP		ds.p	0		; ($1d) ADC Command Base Pointer Register (24 bit)
ADCCBP_0	ds.b	1		; ($1d) ADC Command Base Pointer Register MSB
ADCCBP_1	ds.b	1
ADCCBP_2	ds.b	1		; ($1f) ADC Command Base Pointer Register LSB
ADCRIDX		ds.b	1		; ($20) ADC Result Index Register
RES_IDX		s12zfld	ADCRIDX,6:0
ADCRBP		ds.p	0		; ($21) ADC Result Base Pointer Register (24 bit)
ADCRBP_0	ds.b	1		; ($21) ADC Result Base Pointer Register MSB
ADCRBP_1	ds.b	1
ADCRBP_2	ds.b	1		; ($23) ADC Result Base Pointer Register LSB
ADCCROFF0	ds.b	1		; ($24) ADC Command and Result Offset Register 0
CMDRES_OFF0	s12zfld	ADCCROFF0,7:0	;  ADC Command and Result Offset Value 0
ADCCROFF1	ds.b	1		; ($25) ADC Command and Result Offset Register 1
CMDRES_OFF1	s12zfld	ADCCROFF1,7:0	;  ADC Command and Result Offset Value 1
		ends	S12ZADC

		restore			; re-enable listing

		endif			; __s12z_adc_inc
