		ifndef	__tr24inc
__tr24inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TR24.INC                                                *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega RFR2 Transceiver        *
;*                                                                          *
;****************************************************************************

AES_CTRL	sfr	0x13c		; AES Control Register
AES_REQUEST	avrbit	AES_CTRL,7	; Request AES Operation
AES_MODE	avrbit	AES_CTRL,5	; Set AES Operation Mode
AES_DIR		avrbit	AES_CTRL,3	; Set AES Operation Direction
AES_IM		avrbit	AES_CTRL,2	; AES Interrupt Enable

AES_STATUS	sfr	0x13d		; AES Status Register
AES_ER		avrbit	AES_STATUS,7	; AES Operation Finished with Error
AES_DONE	avrbit	AES_STATUS,0	; AES Operation Finished with Success

AES_STATE	sfr	0x13e		; AES Plain and Cipher Text Buffer Register

AES_KEY		sfr	0x13f		; AES Encryption and Decryption Key Buffer Register

TRX_STATUS	sfr	0x141		; Transceiver Status Register
CCA_DONE	avrbit	TRX_STATUS,7	; CCA Algorithm Status
CCA_STATUS	avrbit	TRX_STATUS,6	; CCA Status Result
TST_STATUS	avrbit	TRX_STATUS,5	; Test Mode Status
TRX_STATUS4	avrbit	TRX_STATUS,4	; Transceiver Main Status
TRX_STATUS3	avrbit	TRX_STATUS,3
TRX_STATUS2	avrbit	TRX_STATUS,2
TRX_STATUS1	avrbit	TRX_STATUS,1
TRX_STATUS0	avrbit	TRX_STATUS,0

TRX_STATE	sfr	0x142		; Transceiver State Control Register
TRAC_STATUS2	avrbit	TRX_STATE,7	; Transaction Status
TRAC_STATUS1	avrbit	TRX_STATE,6
TRAC_STATUS0	avrbit	TRX_STATE,5
TRX_CMD4	avrbit	TRX_STATE,4	; State Control Command
TRX_CMD3	avrbit	TRX_STATE,3
TRX_CMD2	avrbit	TRX_STATE,2
TRX_CMD1	avrbit	TRX_STATE,1
TRX_CMD0	avrbit	TRX_STATE,0

TRX_CTRL_0	sfr	0x143		; Reserved

TRX_CTRL_1	sfr	0x144		; Transceiver Control Register 1
PA_EXT_EN	avrbit	TRX_CTRL_1,7	; External PA Support Enable
IRQ_2_EXT_EN	avrbit	TRX_CTRL_1,6	; Connect Frame Start IRQ to TC1
TX_AUTO_CRC_ON	avrbit	TRX_CTRL_1,5	; Enable Automatic CRC Calculation
PLL_TX_FLT	avrbit	TRX_CTRL_1,4	; Enable PLL TX Filter

PHY_TX_PWR	sfr	0x145		; Transceiver Transmit Power Control Register
TX_PWR3		avrbit	PHY_TX_PWR,3	; Transmit Power Setting
TX_PWR2		avrbit	PHY_TX_PWR,2
TX_PWR1		avrbit	PHY_TX_PWR,1
TX_PWR0		avrbit	PHY_TX_PWR,0

PARCR		sfr	0x138		; Power Amplifier Ramp Up/Down Control Register
PALTD2		avrbit	PARCR,7		; ext. PA Ramp Down Lead Time
PALTD1		avrbit	PARCR,6
PALTD0		avrbit	PARCR,5
PALTU2		avrbit	PARCR,4		; ext. PA Ramp Up Lead Time
PALTU1		avrbit	PARCR,3
PALTU0		avrbit	PARCR,2
PARDFI		avrbit	PARCR,1		; Power Amplifier Ramp Down Frequency Inversion
PARUFI		avrbit	PARCR,0		; Power Amplifier Ramp Up Frequency Inversion

PHY_RSSI	sfr	0x146		; Receiver Signal Strength Indicator Register
RX_CRC_VALID	avrbit	PHY_RSSI,7	; Received Frame CRC Status
RND_VALUE1	avrbit	PHY_RSSI,6	; Random Value
RND_VALUE0	avrbit	PHY_RSSI,5
RSSI4		avrbit	PHY_RSSI,4	; Receiver Signal Strength Indicator
RSSI3		avrbit	PHY_RSSI,3
RSSI2		avrbit	PHY_RSSI,2
RSSI1		avrbit	PHY_RSSI,1
RSSI0		avrbit	PHY_RSSI,0

PHY_ED_LEVEL	sfr	0x147		; Transceiver Energy Detection Level Register
ED_LEVEL7	avrbit	PHY_ED_LEVEL,7	; Energy Detection Level
ED_LEVEL6	avrbit	PHY_ED_LEVEL,6
ED_LEVEL5	avrbit	PHY_ED_LEVEL,5
ED_LEVEL4	avrbit	PHY_ED_LEVEL,4
ED_LEVEL3	avrbit	PHY_ED_LEVEL,3
ED_LEVEL2	avrbit	PHY_ED_LEVEL,2
ED_LEVEL1	avrbit	PHY_ED_LEVEL,1
ED_LEVEL0	avrbit	PHY_ED_LEVEL,0

PHY_CC_CCA	sfr	0x148		; Transceiver Clear Channel Assessment (CCA) Control Register
CCA_REQUEST	avrbit	PHY_CC_CCA,7	; Manual CCA Measurement Request
CCA_MODE1	avrbit	PHY_CC_CCA,6	; Select CCA Measurement Mode
CCA_MODE0	avrbit	PHY_CC_CCA,5
CHANNEL4	avrbit	PHY_CC_CCA,4	; RX/TX Channel Selection
CHANNEL3	avrbit	PHY_CC_CCA,3
CHANNEL2	avrbit	PHY_CC_CCA,2
CHANNEL1	avrbit	PHY_CC_CCA,1
CHANNEL0	avrbit	PHY_CC_CCA,0

CCA_THRES	sfr	0x149		; Transceiver CCA Threshold Setting Register
CCA_CS_THRES3	avrbit	CCA_THRES,7	; CS Threshold Level for CCA Measurement
CCA_CS_THRES2	avrbit	CCA_THRES,6
CCA_CS_THRES1	avrbit	CCA_THRES,5
CCA_CS_THRES0	avrbit	CCA_THRES,4
CCA_ED_THRES3	avrbit	CCA_THRES,3	; ED Threshold Level for CCA Measurement
CCA_ED_THRES2	avrbit	CCA_THRES,2
CCA_ED_THRES1	avrbit	CCA_THRES,1
CCA_ED_THRES0	avrbit	CCA_THRES,0

RX_CTRL		sfr	0x14a		; Transceiver Receive Control Register
SDM_MODE1	avrbit	RX_CTRL,7	; Sigma-Delta Modulator Order and Delay Compensation
SDM_MODE0	avrbit	RX_CTRL,6
ACR_MODE	avrbit	RX_CTRL,5	; Adjacent Channel Rejection Mode
SOFT_MODE	avrbit	RX_CTRL,4	; Correlator Soft Mode
PDT_THRES3	avrbit	RX_CTRL,3	; Receiver Sensitivity Control
PDT_THRES2	avrbit	RX_CTRL,2
PDT_THRES1	avrbit	RX_CTRL,1
PDT_THRES0	avrbit	RX_CTRL,0

SFD_VALUE	sfr	0x14b		; Start of Frame Delimiter Value Register

TRX_CTRL_2	sfr	0x14c		; Transceiver Control Register 2
RX_SAFE_MODE	avrbit	TRX_CTRL_2,7	; RX Safe Mode
OQPSK_DATA_RATE1 avrbit	TRX_CTRL_2,1	; Data Rate Selection
OQPSK_DATA_RATE0 avrbit	TRX_CTRL_2,0

ANT_DIV		sfr	0x14d		; Antenna Diversity Control Register
ANT_SEL		avrbit	ANT_DIV,7	; Antenna Diversity Antenna Status
ANT_DIV_EN	avrbit	ANT_DIV,3	; Enable Antenna Diversity
ANT_EXT_SW_EN	avrbit	ANT_DIV,2	; Enable External Antenna Switch Control
ANT_CTRL1	avrbit	ANT_DIV,1	; Static Antenna Diversity Switch Control
ANT_CTRL0	avrbit	ANT_DIV,0

IRQ_MASK	sfr	0x14e		; Transceiver Interrupt Enable Register
AWAKE_EN	avrbit	IRQ_MASK,7	; Awake Interrupt Enable
TX_END_EN	avrbit	IRQ_MASK,6	; TX_END Interrupt Enable
AMI_EN		avrbit	IRQ_MASK,5	; Address Match Interrupt Enable
CCA_ED_DONE_EN	avrbit	IRQ_MASK,4	; End of ED Measurement Interrupt Enable
RX_END_EN	avrbit	IRQ_MASK,3	; RX_END Interrupt Enable
RX_START_EN	avrbit	IRQ_MASK,2	; RX_START Interrupt Enable
PLL_UNLOCK_EN	avrbit	IRQ_MASK,1	; PLL Unlock Interrupt Enable
PLL_LOCK_EN	avrbit	IRQ_MASK,0	; PLL Lock Interrupt Enable

IRQ_MASK1	sfr	0xbe		; Transceiver Interrupt Enable Register 1
MAF_3_AMI_EN	avrbit	IRQ_MASK1,4	; Address Match Interrupt Enable Address Filter 3
MAF_2_AMI_EN	avrbit	IRQ_MASK1,3	; Address Match Interrupt Enable Address Filter 2
MAF_1_AMI_EN	avrbit	IRQ_MASK1,2	; Address Match Interrupt Enable Address Filter 1
MAF_0_AMI_EN	avrbit	IRQ_MASK1,1	; Address Match Interrupt Enable Address Filter 0
TX_START_EN	avrbit	IRQ_MASK1,0	; Transmit Start Interrupt Enable

IRQ_STATUS	sfr	0x14f		; Transceiver Interrupt Status Register
AWAKE		avrbit	IRQ_STATUS,7	; Awake Interrupt Status
TX_END		avrbit	IRQ_STATUS,6	; TX_END Interrupt Status
AMI		avrbit	IRQ_STATUS,5	; Address Match Interrupt Status
CCA_ED_DONE	avrbit	IRQ_STATUS,4	; End of ED Measurement Interrupt Status
RX_END		avrbit	IRQ_STATUS,3	; RX_END Interrupt Status
RX_START	avrbit	IRQ_STATUS,2	; RX_START Interrupt Status
PLL_UNLOCK	avrbit	IRQ_STATUS,1	; PLL Unlock Interrupt Status
PLL_LOCK	avrbit	IRQ_STATUS,0	; PLL Lock Interrupt Status

IRQ_STATUS1	sfr	0xbf		; Transceiver Interrupt Status Register 1
MAF_3_AMI	avrbit	IRQ_STATUS1,4	; Address Match Interrupt Status Address Filter 3
MAF_2_AMI	avrbit	IRQ_STATUS1,3	; Address Match Interrupt Status Address Filter 2
MAF_1_AMI	avrbit	IRQ_STATUS1,2	; Address Match Interrupt Status Address Filter 1
MAF_0_AMI	avrbit	IRQ_STATUS1,1	; Address Match Interrupt Status Address Filter 0
TX_START	avrbit	IRQ_STATUS1,0	; Transmit Start Interrupt Status

VREG_CTRL	sfr	0x150		; Voltage Regulator Control and Status Register
AVREG_EXT	avrbit	VREG_CTRL,7	; Use External AVDD Regulator
AVDD_OK		avrbit	VREG_CTRL,6	; AVDD Supply Voltage Valid
AVREG_TRIM1	avrbit	VREG_CTRL,5	; Adjust AVDD Supply Voltage
AVREG_TRIM0	avrbit	VREG_CTRL,4
DVREG_EXT	avrbit	VREG_CTRL,3	; Use External DVDD Regulator
DVDD_OK		avrbit	VREG_CTRL,2	; DVDD Supply Voltage Valid
DVREG_TRIM1	avrbit	VREG_CTRL,1	; Adjust DVDD Supply Voltage
DVREG_TRIM0	avrbit	VREG_CTRL,0

BATMON		sfr	0x151		; Battery Monitor Control and Status Register
BAT_LOW		avrbit	BATMON,7	; Battery Monitor Interrupt Status
BAT_LOW_EN	avrbit	BATMON,6	; Battery Monitor Interrupt Enable
BATMON_OK	avrbit	BATMON,5	; Battery Monitor Status
BATMON_HR	avrbit	BATMON,4	; Battery Monitor Voltage Range
BATMON_VTH3	avrbit	BATMON,3	; Battery Monitor Threshold Voltage
BATMON_VTH2	avrbit	BATMON,2
BATMON_VTH1	avrbit	BATMON,1
BATMON_VTH0	avrbit	BATMON,0

XOSC_CTRL	sfr	0x152		; Crystal Oscillator Control Register
XTAL_MODE3	avrbit	XOSC_CTRL,7	; Crystal Oscillator Operating Mode
XTAL_MODE2	avrbit	XOSC_CTRL,6
XTAL_MODE1	avrbit	XOSC_CTRL,5		
XTAL_MODE0	avrbit	XOSC_CTRL,4
XTAL_TRIM3	avrbit	XOSC_CTRL,3	; Crystal Oscillator Load Capacitance Trimming
XTAL_TRIM2	avrbit	XOSC_CTRL,2
XTAL_TRIM1	avrbit	XOSC_CTRL,1
XTAL_TRIM0	avrbit	XOSC_CTRL,0

RX_SYN		sfr	0x155		; Transceiver Receiver Sensitivity Control Register
RX_PDT_DIS	avrbit	RX_SYN,7	; PrEvent Frame Reception
RX_OVERRIDE	avrbit	RX_SYN,6	; Receiver Override Function
RXO_CFG1	avrbit	RX_SYN,5	; RX_OVERRIDE Configuration
RXO_CFG0	avrbit	RX_SYN,4
RX_PDT_LEVEL3	avrbit	RX_SYN,3	; Reduce Receiver Sensitivity
RX_PDT_LEVEL2	avrbit	RX_SYN,2
RX_PDT_LEVEL1	avrbit	RX_SYN,1
RX_PDT_LEVEL0	avrbit	RX_SYN,0

XAH_CTRL_1	sfr	0x157		; Transceiver Acknowledgment Frame Control Register 1
AACK_FLTR_RES_FT avrbit	XAH_CTRL_1,5	; Filter Reserved Frames
AACK_UPLD_RES_FT avrbit	XAH_CTRL_1,4	; Process Reserved Frames
AACK_ACK_TIME	avrbit	XAH_CTRL_1,2	; Reduce Acknowledgment Time
AACK_PROM_MODE	avrbit	XAH_CTRL_1,1	; Enable Promiscuous Mode

FTN_CTRL	sfr	0x158		; Transceiver Filter Tuning Control Register
FTN_START	avrbit	FTN_CTRL,7	; Start Calibration Loop of Filter Tuning Network
FTN_ROUND	avrbit	FTN_CTRL,6	; Round Filter Tuning Calibration Result
FTNV5		avrbit	FTN_CTRL,5	; Filter Tuning Calibration Result
FTNV4		avrbit	FTN_CTRL,4
FTNV3		avrbit	FTN_CTRL,3
FTNV2		avrbit	FTN_CTRL,2
FTNV1		avrbit	FTN_CTRL,1
FTNV0		avrbit	FTN_CTRL,0

PLL_CF		sfr	0x15a		; Transceiver Center Frequency Calibration Control Register
PLL_CF_START	avrbit	PLL_CF,7	; Start Center Frequency Calibration
EN_PLL_CF	avrbit	PLL_CF,6	; Enable Center Frequency Tuning
PLL_VMOD_TUNE1	avrbit	PLL_CF,5	; VCO Modulation Tuning
PLL_VMOD_TUNE0	avrbit	PLL_CF,4
PLL_CF3		avrbit	PLL_CF,3	; Center Frequency Control Word
PLL_CF2		avrbit	PLL_CF,2
PLL_CF1		avrbit	PLL_CF,1
PLL_CF0		avrbit	PLL_CF,0

PLL_DCU		sfr	0x15b		; Transceiver Delay Cell Calibration Control Register
PLL_DCU_START	avrbit	PLL_DCU,7	; Start Delay Cell Calibration
PLL_DCUW5	avrbit	PLL_DCU,5	; Delay Range Setting
PLL_DCUW4	avrbit	PLL_DCU,4
PLL_DCUW3	avrbit	PLL_DCU,3
PLL_DCUW2	avrbit	PLL_DCU,2
PLL_DCUW1	avrbit	PLL_DCU,1
PLL_DCUW0	avrbit	PLL_DCU,0

CC_CTRL_0	sfr	0x153		; Channel Control Register 0
CC_NUMBER7	avrbit	CC_NUMBER7,7	; Channel Number
CC_NUMBER6	avrbit	CC_NUMBER7,6
CC_NUMBER5	avrbit	CC_NUMBER7,5
CC_NUMBER4	avrbit	CC_NUMBER7,4
CC_NUMBER3	avrbit	CC_NUMBER7,3
CC_NUMBER2	avrbit	CC_NUMBER7,2
CC_NUMBER1	avrbit	CC_NUMBER7,1
CC_NUMBER0	avrbit	CC_NUMBER7,0

CC_CTRL_1	sfr	0x154		; Channel Control Register 1
CC_BAND3	avrbit	CC_CTRL_1,3	; Channel Band
CC_BAND2	avrbit	CC_CTRL_1,2
CC_BAND1	avrbit	CC_CTRL_1,1
CC_BAND0	avrbit	CC_CTRL_1,0

TRX_RPC		sfr	0x156		; Transceiver Reduced Power Consumption Control
RX_RPC_CTRL1	avrbit	TRX_RPC,7	; Smart Receiving Mode Timing
RX_RPC_CTRL0	avrbit	TRX_RPC,6
RX_RPC_EN	avrbit	TRX_RPC,5	; Receiver Smart Receiving Mode Enable
PDT_RPC_EN	avrbit	TRX_RPC,4	; Smart Receiving Mode Reduced Sensitivity Enable
PLL_RPC_EN	avrbit	TRX_RPC,3	; PLL Smart Receiving Mode Enable
IPAN_RPC_EN	avrbit	TRX_RPC,1	; Smart Receiving Mode IPAN Handling Enable
XAH_RPC_EN	avrbit	TRX_RPC,0	; Smart Receiving in Extended Operating Modes Enable

PART_NUM	sfr	0x15c		; Device Identification Register (Part Number)

VERSION_NUM	sfr	0x15d		; Device Identification Register (Version Number)

MAN_ID_0	sfr	0x15e		; Device Identification Register (Manufacture ID Low Byte)
MAN_ID_1	sfr	0x15f		; Device Identification Register (Manufacture ID High Byte)

SHORT_ADDR_0	sfr	0x160		; Transceiver MAC Short Address Register (Low Byte)
SHORT_ADDR_1	sfr	0x161		; Transceiver MAC Short Address Register (High Byte)

PAN_ID_0	sfr	0x162		; Transceiver Personal Area Network ID Register (Low Byte)
PAN_ID_1	sfr	0x163		; Transceiver Personal Area Network ID Register (High Byte)

IEEE_ADDR_0	sfr	0x164		; Transceiver MAC IEEE Address Register
IEEE_ADDR_1	sfr	0x165
IEEE_ADDR_2	sfr	0x166
IEEE_ADDR_3	sfr	0x167
IEEE_ADDR_4	sfr	0x168
IEEE_ADDR_5	sfr	0x169
IEEE_ADDR_6	sfr	0x16a
IEEE_ADDR_7	sfr	0x16b

XAH_CTRL_0	sfr	0x16c		; Transceiver Extended Operating Mode Control Register
MAX_FRAME_RETRIES3 avrbit	XAH_CTRL_0,7	; Maximum Number of Frame Retransmission Attempts
MAX_FRAME_RETRIES2 avrbit	XAH_CTRL_0,6
MAX_FRAME_RETRIES1 avrbit	XAH_CTRL_0,5
MAX_FRAME_RETRIES0 avrbit	XAH_CTRL_0,4
MAX_CSMA_RETRIES2 avrbit	XAH_CTRL_0,3	; Maximum Number of CSMA-CA Procedure Repetition Attempts
MAX_CSMA_RETRIES1 avrbit	XAH_CTRL_0,2
MAX_CSMA_RETRIES0 avrbit	XAH_CTRL_0,1
SLOTTED_OPERATION avrbit	XAH_CTRL_0,0	; Set Slotted Acknowledgment

CSMA_SEED_0	sfr	0x16d		; Transceiver CSMA-CA Random Number Generator Seed Register

CSMA_SEED_1	sfr	0x16e		; Transceiver Acknowledgment Frame Control Register 2
AACK_FVN_MODE1	avrbit	CSMA_SEED_1,7	; Acknowledgment Frame Filter Mode
AACK_FVN_MODE0	avrbit	CSMA_SEED_1,6
AACK_SET_PD	avrbit	CSMA_SEED_1,5	; Set Frame Pending Sub-field
AACK_DIS_ACK	avrbit	CSMA_SEED_1,4	; Disable Acknowledgment Frame Transmission
AACK_I_AM_COORD avrbit	CSMA_SEED_1,3	; Set Personal Area Network Coordinator
CSMA_SEED_12	avrbit	CSMA_SEED_1,2	; Seed Value for CSMA Random Number Generator
CSMA_SEED_11	avrbit	CSMA_SEED_1,1
CSMA_SEED_10	avrbit	CSMA_SEED_1,0

CSMA_BE		sfr	0x16f		; Transceiver CSMA-CA Back-off Exponent Control Register
MAX_BE3		avrbit	CSMA_BE,7	; Maximum Back-off Exponent
MAX_BE2		avrbit	CSMA_BE,6
MAX_BE1		avrbit	CSMA_BE,5
MAX_BE0		avrbit	CSMA_BE,4
MIN_BE3		avrbit	CSMA_BE,3	; Minimum Back-off Exponent
MIN_BE2		avrbit	CSMA_BE,2
MIN_BE1		avrbit	CSMA_BE,1
MIN_BE0		avrbit	CSMA_BE,0

MAFCR0		sfr	0x10c		; Multiple Address Filter Configuration Register 0
MAF3EN		avrbit	MAFCR0,3	; Multiple Address Filter 3 Enable
MAF2EN		avrbit	MAFCR0,2	; Multiple Address Filter 2 Enable
MAF1EN		avrbit	MAFCR0,1	; Multiple Address Filter 1 Enable
MAF0EN		avrbit	MAFCR0,0	; Multiple Address Filter 0 Enable

MAFCR1		sfr	0x10d		; Multiple Address Filter Configuration Register 1
AACK_3_SET_PD	avrbit	MAFCR1,7		; Set Data Pending bit for address Filter 3
AACK_3_I_AM_COORD avrbit	MAFCR1,6	; Enable PAN Coordinator Mode for address Filter 3
AACK_2_SET_PD	avrbit	MAFCR1,5		; Set Data Pending bit for address Filter 2
AACK_2_I_AM_COORD avrbit	MAFCR1,4	; Enable PAN Coordinator Mode for address Filter 2
AACK_1_SET_PD	avrbit	MAFCR1,3		; Set Data Pending bit for address Filter 1
AACK_1_I_AM_COORD avrbit	MAFCR1,2	; Enable PAN Coordinator Mode for address Filter 1
AACK_0_SET_PD	avrbit	MAFCR1,1		; Set Data Pending bit for address Filter 0
AACK_0_I_AM_COORD avrbit	MAFCR1,0	; Enable PAN Coordinator Mode for address Filter 0

MAFPA0H		sfr	0x111		; Transceiver Personal Area Network ID Register for Frame Filter 0 (High Byte)
MAFPA0L		sfr	0x110		; Transceiver Personal Area Network ID Register for Frame Filter 0 (Low Byte)
MAFPA1H		sfr	0x115		; Transceiver Personal Area Network ID Register for Frame Filter 1 (High Byte)
MAFPA1L		sfr	0x114		; Transceiver Personal Area Network ID Register for Frame Filter 1 (Low Byte)
MAFPA2H		sfr	0x119		; Transceiver Personal Area Network ID Register for Frame Filter 2 (High Byte)
MAFPA2L		sfr	0x118		; Transceiver Personal Area Network ID Register for Frame Filter 2 (Low Byte)
MAFPA3H		sfr	0x11d		; Transceiver Personal Area Network ID Register for Frame Filter 3 (High Byte)
MAFPA3L		sfr	0x11c		; Transceiver Personal Area Network ID Register for Frame Filter 3 (Low Byte)

MAFSA0H		sfr	0x10f		; Transceiver MAC Short Address Register for Frame Filter 0 (High Byte)
MAFSA0L		sfr	0x10e		; Transceiver MAC Short Address Register for Frame Filter 0 (Low Byte)
MAFSA1H		sfr	0x113		; Transceiver MAC Short Address Register for Frame Filter 1 (High Byte)
MAFSA1L		sfr	0x112		; Transceiver MAC Short Address Register for Frame Filter 1 (Low Byte)
MAFSA2H		sfr	0x117		; Transceiver MAC Short Address Register for Frame Filter 2 (High Byte)
MAFSA2L		sfr	0x116		; Transceiver MAC Short Address Register for Frame Filter 2 (Low Byte)
MAFSA3H		sfr	0x11b		; Transceiver MAC Short Address Register for Frame Filter 3 (High Byte)
MAFSA3L		sfr	0x11a		; Transceiver MAC Short Address Register for Frame Filter 3 (Low Byte)

TST_CTRL_DIGI	sfr	0x176		; Transceiver Digital Test Control Register
TST_CTRL_DIG_7	avrbit	TST_CTRL_DIGI,7	; Disable Receiver Baseband Frequency Synthesis
TST_CTRL_DIG_6	avrbit	TST_CTRL_DIGI,6	; Disable Receiver Baseband Drift Compensation
TST_CTRL_DIG_5	avrbit	TST_CTRL_DIGI,5	; Enable Switch of Transceiver FIFO
TST_CTRL_DIG_4	avrbit	TST_CTRL_DIGI,4	; Switch Receiver Input Data
TST_CTRL_DIG3	avrbit	TST_CTRL_DIGI,3	; Digital Test Controller Register
TST_CTRL_DIG2	avrbit	TST_CTRL_DIGI,2
TST_CTRL_DIG1	avrbit	TST_CTRL_DIGI,1
TST_CTRL_DIG0	avrbit	TST_CTRL_DIGI,0

TST_RX_LENGTH	sfr	0x17b		; Transceiver Received Frame Length Register

TRXFBST		sfr	0x180		; Start of Frame Buffer

TRXFBEND	sfr	0x1ff		; End of Frame Buffer

TRXPR		sfr	0x139		; Transceiver Pin Register
ATBE		avrbit	TRXPR,3		; Analog Test-Bus Enable
TRXTST		avrbit	TRXPR,2		; Transceiver Test-Mode Enable
SLPTR		avrbit	TRXPR,1		; Multi-Purpose Transceiver Control Bit
TRXRST		avrbit	TRXPR,0		; Force Transceiver Reset

		restore			; re-enable listing

		endif			; __tr24inc
