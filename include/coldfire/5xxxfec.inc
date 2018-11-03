		ifndef	__5xxxfecinc		; avoid multiple inclusion
__5xxxfecinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 5XXXFEC.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5xxx Fast Ethernet    *
;*   Controller                                                             *
;*                                                                          *
;****************************************************************************

__deffec	macro		PR,Base
{PR}EIR		equ		Base+$004	; Interrupt Event Register (32b)
HBERR		cfbit		{PR}EIR,31	;  Heartbeat Error
BABR		cfbit		{PR}EIR,30	;  Babbling Receive Error
BABT		cfbit		{PR}EIR,29	;  Babbling Transmit Error
GRA		cfbit		{PR}EIR,28	;  Graceful Stop Complete
TXF		cfbit		{PR}EIR,27	;  Transmit Frame Interrupt
MII		cfbit		{PR}EIR,23	;  MII Interrupt
LC		cfbit		{PR}EIR,21	;  Late Collision
RL		cfbit		{PR}EIR,20	;  Collision Retry Limit
{PR}EIMR	equ		Base+$008	; Interrupt Mask Register (32b)
HBERR		cfbit		{PR}EIMR,31	;  Heartbeat Error
BABR		cfbit		{PR}EIMR,30	;  Babbling Receive Error
BABT		cfbit		{PR}EIMR,29	;  Babbling Transmit Error
GRA		cfbit		{PR}EIMR,28	;  Graceful Stop Complete
TXF		cfbit		{PR}EIMR,27	;  Transmit Frame Interrupt
MII		cfbit		{PR}EIMR,23	;  MII Interrupt
LC		cfbit		{PR}EIMR,21	;  Late Collision
RL		cfbit		{PR}EIMR,20	;  Collision Retry Limit
{PR}ECR		equ		Base+$024	; Ethernet Control Register (32b)
ETHER_EN	cfbit		{PR}ECR,1	;  Enable FEC
RESET		cfbit		{PR}ECR,0	;  Hardware Reset
{PR}MMFR	equ		Base+$040	; MII Management Frame Register (32b)
ST		cffield		{PR}MMFR,30,2	;  Start of Frame Delimiter
OP		cffield		{PR}MMFR,28,2	;  Operation Code
PA		cffield		{PR}MMFR,23,5	;  PHY Address
RA		cffield		{PR}MMFR,18,5	;  Register Address
TA		cffield		{PR}MMFR,16,2	;  Turn Around
DATA		cffield		{PR}MMFR,0,16	;  Management Frame Data
{PR}MSCR	equ		Base+$044	; MII Speed Control Register (32b)
DIS_PRE		cfbit		{PR}MSCR,7	;  Disable Preamble
MII_SPEED	cffield		{PR}MSCR,1,5	;  MII Clock
{PR}MIBC	equ		Base+$064	; MIB Control/Status Register (32b)
MIB_DIS		cfbit		{PR}MIBC,31	;  Halt Counters
MIB_IDLE	cfbit		{PR}MIBC,30	;  Counters Idle?
{PR}FEC_RCR	equ		Base+$084	; Receive Control Register (32b)
MAX_FL		cffield		{PR}FEC_RCR,16,11;  Maximum Frame Length
FCE		cfbit		{PR}FEC_RCR,5	;  Flow control enable
BC_REJ		cfbit		{PR}FEC_RCR,4	;  Broadcast Frame Reject
PROM		cfbit		{PR}FEC_RCR,3	;  Promiscuous Mode
MII_MODE	cfbit		{PR}FEC_RCR,2	;  Media Independent Interface Mode
DRT		cfbit		{PR}FEC_RCR,1	;  Disable Receive on Transmit
LOOP		cfbit		{PR}FEC_RCR,0	;  Internal Loopback
{PR}TCR		equ		Base+$0C4	; Transmit Control Register (32b)
RFC_PAUSE	cfbit		{PR}TCR,4	;  Receive Frame Control Pause
TFC_PAUSE	cfbit		{PR}TCR,3	;  Transmit Frame Control Pause
FDEN		cfbit		{PR}TCR,2	;  Full Duplex Enable
HBC		cfbit		{PR}TCR,1	;  Heartbeat Control
GTS		cfbit		{PR}TCR,0	;  Graceful Transmit Stop
{PR}PALR	equ		Base+$0E4	; Physical Address Low Register (32b)
PADDR1		cffield		{PR}PALR,0,32	;  bits 31..0 of MAC address
{PR}PAUR	equ		Base+$0E8	; Physical Address High Register (32b)
PADDR2		cffield		{PR}PAUR,16,16	;  bits 47..32 of MAC address
TYPE		cffield		{PR}PAUR0,016	;  Ethertype in PAUSE frames (0x8808)
{PR}OPD		equ		Base+$0EC	; Opcode/Pause Duration (32b)
OPCODE		cffield		{PR}OPD,16,16	;  Opcode Field Used in PAUSE Frames
PAUSE_DUR	cffield		{PR}OPD,0,16	;  Pause Duration Field Used in PAUSE Frames
{PR}IAUR	equ		Base+$118	; Descriptor Individual Upper Address Register (32b)
{PR}IALR	equ		Base+$11C	; Descriptor Individual Lower Address Register (32b)
{PR}GAUR	equ		Base+$120	; Descriptor Group Upper Address Register (32b)
{PR}GALR	equ		Base+$124	; Descriptor Group Lower Address Register (32b)
{PR}TFWR	equ		Base+$144	; Transmit FIFO Watermark (32b)

{PR}RMON_T_DROP	equ		Base+$200	; Count of Frames not Counted Correctly
{PR}RMON_T_PACKETS	equ	Base+$204	; RMON Tx Packet count
{PR}RMON_T_BC_PKT	equ	Base+$208	; RMON Tx broadcast Packets
{PR}RMON_T_MC_PKT	equ	Base+$20C	; RMON Tx multicast Packets
{PR}RMON_T_CRC_ALIGN equ	Base+$210	; RMON Tx Packets With CRC/Align Error
{PR}RMON_T_UNDERSIZE equ	Base+$214	; RMON Tx Packets < 64 Bytes, Good CRC
{PR}RMON_T_OVERSIZE	equ	Base+$218	; RMON Tx Packets > MAX_FL Bytes, Good CRC
{PR}RMON_T_FRAG	equ		Base+$21C	; RMON Tx Packets < 64 Bytes, Bad CRC
{PR}RMON_T_JAB	equ		Base+$220	; RMON Tx Packets > MAX_FL Bytes, Bad CRC
{PR}RMON_T_COL	equ		Base+$224	; RMON Tx Collision Count
{PR}RMON_T_P64	equ		Base+$228	; RMON Tx 64 Byte Packets
{PR}RMON_T_P65TO127	equ	Base+$22C	; RMON Tx 65 to 127 Byte Packets
{PR}RMON_T_P128TO255 equ	Base+$230	; RMON Tx 128 to 255 Byte Packets
{PR}RMON_T_P256TO511 equ	Base+$234	; RMON Tx 256 to 511 Byte Packets
{PR}RMON_T_P512TO1023 equ	Base+$238	; RMON Tx 512 to 1023 Byte Packets
{PR}RMON_T_P1024TO2047 equ	Base+$23C	; RMON Tx 1024 to 2047 Byte Packets
{PR}RMON_T_P_GTE2048 equ	Base+$240	; RMON Tx Packets With > 2048 Bytes
{PR}RMON_T_OCTETS	equ	Base+$244	; RMON Tx Octets
{PR}IEEE_T_DROP	equ		Base+$248	; Count of Transmitted Frames not Counted Correctly
{PR}IEEE_T_FRAME_OK	equ	Base+$24C	; Frames Transmitted OK
{PR}IEEE_T_1COL	equ		Base+$250	; Frames Transmitted With Single Collision
{PR}IEEE_T_MCOL	equ		Base+$254	; Frames Transmitted With Multiple Collisions
{PR}IEEE_T_DEF	equ		Base+$258	; Frames Transmitted after Deferral Delay
{PR}IEEE_T_LCOL	equ		Base+$25C	; Frames Transmitted With Late Collision
{PR}IEEE_T_EXCOL	equ	Base+$260	; Frames Transmitted With Excessive Collisions
{PR}IEEE_T_MACERR	equ	Base+$264	; Frames Transmitted With Tx FIFO Underrun
{PR}IEEE_T_CSERR	equ	Base+$268	; Frames Transmitted With Carrier Sense Error
{PR}IEEE_T_SQE	equ		Base+$26C	; Frames Transmitted With SQE Error
{PR}IEEE_T_FDXFC	equ	Base+$270	; Flow control pause Frames Transmitted
{PR}IEEE_T_OCTETS_OK equ	Base+$274	; Octet Count for Frames Transmitted Without Error
{PR}RMON_R_DROP	equ		Base+$280	; Count of Received Frames not Counted Correctly
{PR}RMON_R_PACKETS	equ	Base+$284	; RMON Rx Packet Count
{PR}RMON_R_BC_PKT	equ	Base+$288	; RMON Rx Broadcast Packets
{PR}RMON_R_MC_PKT	equ	Base+$28C	; RMON Rx Multicast Packets
{PR}RMON_R_CRC_ALIGN equ	Base+$290	; RMON Rx Packets With CRC/Align Error
{PR}RMON_R_UNDERSIZE equ	Base+$294	; RMON Rx Packets < 64 Bytes, Good CRC
{PR}RMON_R_OVERSIZE	equ	Base+$298	; RMON Rx Packets > MAX_FL Bytes, Good CRC
{PR}RMON_R_FRAG	equ		Base+$29C	; RMON Rx Packets < 64 Bytes, Bad CRC
{PR}RMON_R_JAB	equ		Base+$2A0	; RMON Rx Packets > MAX_FL Bytes, Bad CRC
{PR}RMON_R_RESVD_0	equ	Base+$2A4	; Reserved
{PR}RMON_R_P64	equ		Base+$2A8	; RMON Rx 64 Byte Packets
{PR}RMON_R_P65TO127	equ	Base+$2AC	; RMON Rx 65 to 127 Byte Packets
{PR}RMON_R_P128TO255 equ	Base+$2B0	; RMON Rx 128 to 255 Byte Packets
{PR}RMON_R_P256TO511 equ	Base+$2B4	; RMON Rx 256 to 511 Byte Packets
{PR}RMON_R_P512TO1023 equ	Base+$2B8	; RMON Rx 512 to 1023 Byte Packets
{PR}RMON_R_P1024TO2047 equ	Base+$2BC	; RMON Rx 1024 to 2047 Byte Packets
{PR}RMON_R_P_GTE2048 equ	Base+$2C0	; RMON Rx Packets With > 2048 Bytes
{PR}RMON_R_OCTETS	equ	Base+$2C4	; RMON Rx octets
{PR}IEEE_R_DROP	equ		Base+$2C8	; Count of Received Frames not Counted Correctly
{PR}IEEE_R_FRAME_OK	equ	Base+$2CC	; Frames Received OK
{PR}IEEE_R_CRC	equ		Base+$2D0	; Frames Received With CRC Error
{PR}IEEE_R_ALIGN	equ	Base+$2D4	; Frames Received With Alignment Error
{PR}IEEE_R_MACERR	equ	Base+$2D8	; Receive FIFO Overflow Count
{PR}IEEE_R_FDXFC	equ	Base+$2DC	; Flow Control Pause Frames Received
{PR}IEEE_R_OCTETS_OK equ	Base+$2E0	; Octet Count for Frames Received Without Error
		endm

		restore				; re-enable listing

                endif                           ; __5xxxfecinc
