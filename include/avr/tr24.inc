		ifndef	__tr24inc
__tr24inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TR24.INC                                                *
;*                                                                          *
;*   Contains bit & register definitions for ATmega RFR2 Transceiver        *
;*                                                                          *
;****************************************************************************

AES_CTRL	sfr	0x13c		; AES Control Register
AES_REQUEST	equ	7		; Request AES Operation
AES_MODE	equ	5		; Set AES Operation Mode
AES_DIR		equ	3		; Set AES Operation Direction
AES_IM		equ	2		; AES Interrupt Enable

AES_STATUS	sfr	0x13d		; AES Status Register
AES_ER		equ	7		; AES Operation Finished with Error
AES_DONE	equ	0		; AES Operation Finished with Success

AES_STATE	sfr	0x13e		; AES Plain and Cipher Text Buffer Register

AES_KEY		sfr	0x13f		; AES Encryption and Decryption Key Buffer Register

TRX_STATUS	sfr	0x141		; Transceiver Status Register
CCA_DONE	equ	7		; CCA Algorithm Status
CCA_STATUS	equ	6		; CCA Status Result
TST_STATUS	equ	5		; Test mode status
TRX_STATUS4	equ	4		; Transceiver Main Status
TRX_STATUS3	equ	3
TRX_STATUS2	equ	2
TRX_STATUS1	equ	1
TRX_STATUS0	equ	0

TRX_STATE	sfr	0x142		; Transceiver State Control Register
TRAC_STATUS2	equ	7		; Transaction Status
TRAC_STATUS1	equ	6
TRAC_STATUS0	equ	5
TRX_CMD4	equ	4		; State Control Command
TRX_CMD3	equ	3
TRX_CMD2	equ	2
TRX_CMD1	equ	1
TRX_CMD0	equ	0

TRX_CTRL_0	sfr	0x143		; Reserved

TRX_CTRL_1	sfr	0x144		; Transceiver Control Register 1
PA_EXT_EN	equ	7		; External PA support enable
IRQ_2_EXT_EN	equ	6		; Connect Frame Start IRQ to TC1
TX_AUTO_CRC_ON	equ	5		; Enable Automatic CRC Calculation
PLL_TX_FLT	equ	4		; Enable PLL TX Filter

PHY_TX_PWR	sfr	0x145		; Transceiver Transmit Power Control Register
TX_PWR3		equ	3		; Transmit Power Setting
TX_PWR2		equ	2
TX_PWR1		equ	1
TX_PWR0		equ	0

PARCR		sfr	0x138		; Power Amplifier Ramp up/down Control Register
PALTD2		equ	7		; ext. PA Ramp Down Lead Time
PALTD1		equ	6
PALTD0		equ	5
PALTU2		equ	4		; ext. PA Ramp Up Lead Time
PALTU1		equ	3
PALTU0		equ	2
PARDFI		equ	1		; Power Amplifier Ramp Down Frequency Inversion
PARUFI		equ	0		; Power Amplifier Ramp Up Frequency Inversion

PHY_RSSI	sfr	0x146		; Receiver Signal Strength Indicator Register
RX_CRC_VALID	equ	7		; Received Frame CRC Status
RND_VALUE1	equ	6		; Random Value
RND_VALUE0	equ	5
RSSI4		equ	4		; Receiver Signal Strength Indicator
RSSI3		equ	3
RSSI2		equ	2
RSSI1		equ	1
RSSI0		equ	0

PHY_ED_LEVEL	sfr	0x147		; Transceiver Energy Detection Level Register
ED_LEVEL7	equ	7		; Energy Detection Level
ED_LEVEL6	equ	6
ED_LEVEL5	equ	5
ED_LEVEL4	equ	4
ED_LEVEL3	equ	3
ED_LEVEL2	equ	2
ED_LEVEL1	equ	1
ED_LEVEL0	equ	0

PHY_CC_CCA	sfr	0x148		; Transceiver Clear Channel Assessment (CCA) Control Register
CCA_REQUEST	equ	7		; Manual CCA Measurement Request
CCA_MODE1	equ	6		; Select CCA Measurement Mode
CCA_MODE0	equ	5
CHANNEL4	equ	4		; RX/TX Channel Selection
CHANNEL3	equ	3
CHANNEL2	equ	2
CHANNEL1	equ	1
CHANNEL0	equ	0

CCA_THRES	sfr	0x149		; Transceiver CCA Threshold Setting Register
CCA_CS_THRES3	equ	7		; CS Threshold Level for CCA Measurement
CCA_CS_THRES2	equ	6
CCA_CS_THRES1	equ	5
CCA_CS_THRES0	equ	4
CCA_ED_THRES3	equ	3		; ED Threshold Level for CCA Measurement
CCA_ED_THRES2	equ	2
CCA_ED_THRES1	equ	1
CCA_ED_THRES0	equ	0

RX_CTRL		sfr	0x14a		; Transceiver Receive Control Register
SDM_MODE1	equ	7		; Sigma-Delta Modulator Order and Delay Compensation
SDM_MODE0	equ	6
ACR_MODE	equ	5		; Adjacent Channel Rejection Mode
SOFT_MODE	equ	4		; Correlator Soft Mode
PDT_THRES3	equ	3		; Receiver Sensitivity Control
PDT_THRES2	equ	2
PDT_THRES1	equ	1
PDT_THRES0	equ	0

SFD_VALUE	sfr	0x14b		; Start of Frame Delimiter Value Register

TRX_CTRL_2	sfr	0x14c		; Transceiver Control Register 2
RX_SAFE_MODE	equ	7		; RX Safe Mode
OQPSK_DATA_RATE1 equ	1		; Data Rate Selection
OQPSK_DATA_RATE0 equ	0

ANT_DIV		sfr	0x14d		; Antenna Diversity Control Register
ANT_SEL		equ	7		; Antenna Diversity Antenna Status
ANT_DIV_EN	equ	3		; Enable Antenna Diversity
ANT_EXT_SW_EN	equ	2		; Enable External Antenna Switch Control
ANT_CTRL1	equ	1		; Static Antenna Diversity Switch Control
ANT_CTRL0	equ	0

IRQ_MASK	sfr	0x14e		; Transceiver Interrupt Enable Register
AWAKE_EN	equ	7		; Awake Interrupt Enable
TX_END_EN	equ	6		; TX_END Interrupt Enable
AMI_EN		equ	5		; Address Match Interrupt Enable
CCA_ED_DONE_EN	equ	4		; End of ED Measurement Interrupt Enable
RX_END_EN	equ	3		; RX_END Interrupt Enable
RX_START_EN	equ	2		; RX_START Interrupt Enable
PLL_UNLOCK_EN	equ	1		; PLL Unlock Interrupt Enable
PLL_LOCK_EN	equ	0		; PLL Lock Interrupt Enable

IRQ_MASK1	sfr	0xbe		; Transceiver Interrupt Enable Register 1
MAF_3_AMI_EN	equ	4		; Address Match Interrupt enable Address filter 3
MAF_2_AMI_EN	equ	3		; Address Match Interrupt enable Address filter 2
MAF_1_AMI_EN	equ	2		; Address Match Interrupt enable Address filter 1
MAF_0_AMI_EN	equ	1		; Address Match Interrupt enable Address filter 0
TX_START_EN	equ	0		; Transmit Start Interrupt enable

IRQ_STATUS	sfr	0x14f		; Transceiver Interrupt Status Register
AWAKE		equ	7		; Awake Interrupt Status
TX_END		equ	6		; TX_END Interrupt Status
AMI		equ	5		; Address Match Interrupt Status
CCA_ED_DONE	equ	4		; End of ED Measurement Interrupt Status
RX_END		equ	3		; RX_END Interrupt Status
RX_START	equ	2		; RX_START Interrupt Status
PLL_UNLOCK	equ	1		; PLL Unlock Interrupt Status
PLL_LOCK	equ	0		; PLL Lock Interrupt Status

IRQ_STATUS1	sfr	0xbf		; Transceiver Interrupt Status Register 1
MAF_3_AMI	equ	4		; Address Match Interrupt Status Address filter 3
MAF_2_AMI	equ	3		; Address Match Interrupt Status Address filter 2
MAF_1_AMI	equ	2		; Address Match Interrupt Status Address filter 1
MAF_0_AMI	equ	1		; Address Match Interrupt Status Address filter 0
TX_START	equ	0		; Transmit Start Interrupt Status

VREG_CTRL	sfr	0x150		; Voltage Regulator Control and Status Register
AVREG_EXT	equ	7		; Use External AVDD Regulator
AVDD_OK		equ	6		; AVDD Supply Voltage Valid
AVREG_TRIM1	equ	5		; Adjust AVDD Supply Voltage
AVREG_TRIM0	equ	4
DVREG_EXT	equ	3		; Use External DVDD Regulator
DVDD_OK		equ	2		; DVDD Supply Voltage Valid
DVREG_TRIM1	equ	1		; Adjust DVDD Supply Voltage
DVREG_TRIM0	equ	0

BATMON		sfr	0x151		; Battery Monitor Control and Status Register
BAT_LOW		equ	7		; Battery Monitor Interrupt Status
BAT_LOW_EN	equ	6		; Battery Monitor Interrupt Enable
BATMON_OK	equ	5		; Battery Monitor Status
BATMON_HR	equ	4		; Battery Monitor Voltage Range
BATMON_VTH3	equ	3		; Battery Monitor Threshold Voltage
BATMON_VTH2	equ	2
BATMON_VTH1	equ	1
BATMON_VTH0	equ	0

XOSC_CTRL	sfr	0x152		; Crystal Oscillator Control Register
XTAL_MODE3	equ	7		; Crystal Oscillator Operating Mode
XTAL_MODE2	equ	6
XTAL_MODE1	equ	5		
XTAL_MODE0	equ	4
XTAL_TRIM3	equ	3		; Crystal Oscillator Load Capacitance Trimming
XTAL_TRIM2	equ	2
XTAL_TRIM1	equ	1
XTAL_TRIM0	equ	0

RX_SYN		sfr	0x155		; Transceiver Receiver Sensitivity Control Register
RX_PDT_DIS	equ	7		; Prevent Frame Reception
RX_OVERRIDE	equ	6		; Receiver Override Function
RXO_CFG1	equ	5		; RX_OVERRIDE Configuration
RXO_CFG0	equ	4
RX_PDT_LEVEL3	equ	3		; Reduce Receiver Sensitivity
RX_PDT_LEVEL2	equ	2
RX_PDT_LEVEL1	equ	1
RX_PDT_LEVEL0	equ	0

XAH_CTRL_1	sfr	0x157		; Transceiver Acknowledgment Frame Control Register 1
AACK_FLTR_RES_FT equ	5		; Filter Reserved Frames
AACK_UPLD_RES_FT equ	4		; Process Reserved Frames
AACK_ACK_TIME	equ	2		; Reduce Acknowledgment Time
AACK_PROM_MODE	equ	1		; Enable Promiscuous Mode

FTN_CTRL	sfr	0x158		; Transceiver Filter Tuning Control Register
FTN_START	equ	7		; Start Calibration Loop of Filter Tuning Network
FTN_ROUND	equ	6		; Round Filter Tuning Calibration Result
FTNV5		equ	5		; Filter Tuning Calibration Result
FTNV4		equ	4
FTNV3		equ	3
FTNV2		equ	2
FTNV1		equ	1
FTNV0		equ	0

PLL_CF		sfr	0x15a		; Transceiver Center Frequency Calibration Control Register
PLL_CF_START	equ	7		; Start Center Frequency Calibration
EN_PLL_CF	equ	6		; Enable Center Frequency Tuning
PLL_VMOD_TUNE1	equ	5		; VCO Modulation Tuning
PLL_VMOD_TUNE0	equ	4
PLL_CF3		equ	3		; Center Frequency Control Word
PLL_CF2		equ	2
PLL_CF1		equ	1
PLL_CF0		equ	0

PLL_DCU		sfr	0x15b		; Transceiver Delay Cell Calibration Control Register
PLL_DCU_START	equ	7		; Start Delay Cell Calibration
PLL_DCUW5	equ	5		; Delay Range Setting
PLL_DCUW4	equ	4
PLL_DCUW3	equ	3
PLL_DCUW2	equ	2
PLL_DCUW1	equ	1
PLL_DCUW0	equ	0

CC_CTRL_0	sfr	0x153		; Channel Control Register 0
CC_NUMBER7	equ	7		; Channel Number
CC_NUMBER6	equ	6
CC_NUMBER5	equ	5
CC_NUMBER4	equ	4
CC_NUMBER3	equ	3
CC_NUMBER2	equ	2
CC_NUMBER1	equ	1
CC_NUMBER0	equ	0

CC_CTRL_1	sfr	0x154		; Channel Control Register 1
CC_BAND3	equ	3		; Channel Band
CC_BAND2	equ	2
CC_BAND1	equ	1
CC_BAND0	equ	0

TRX_RPC		sfr	0x156		; Transceiver Reduced Power Consumption Control
RX_RPC_CTRL1	equ	7		; Smart Receiving Mode Timing
RX_RPC_CTRL0	equ	6
RX_RPC_EN	equ	5		; Receiver Smart Receiving Mode Enable
PDT_RPC_EN	equ	4		; Smart Receiving Mode Reduced Sensitivity Enable
PLL_RPC_EN	equ	3		; PLL Smart Receiving Mode Enable
IPAN_RPC_EN	equ	1		; Smart Receiving Mode IPAN Handling Enable
XAH_RPC_EN	equ	0		; Smart Receiving in Extended Operating Modes Enable

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
MAX_FRAME_RETRIES3 equ	7		; Maximum Number of Frame Retransmission Attempts
MAX_FRAME_RETRIES2 equ	6
MAX_FRAME_RETRIES1 equ	5
MAX_FRAME_RETRIES0 equ	4
MAX_CSMA_RETRIES2 equ	3		; Maximum Number of CSMA-CA Procedure Repetition Attempts
MAX_CSMA_RETRIES1 equ	2
MAX_CSMA_RETRIES0 equ	1
SLOTTED_OPERATION equ	0		; Set Slotted Acknowledgment

CSMA_SEED_0	sfr	0x16d		; Transceiver CSMA-CA Random Number Generator Seed Register

CSMA_SEED_1	sfr	0x16e		; Transceiver Acknowledgment Frame Control Register 2
AACK_FVN_MODE1	equ	7		; Acknowledgment Frame Filter Mode
AACK_FVN_MODE0	equ	6
AACK_SET_PD	equ	5		; Set Frame Pending Sub-field
AACK_DIS_ACK	equ	4		; Disable Acknowledgment Frame Transmission
AACK_I_AM_COORD equ	3		; Set Personal Area Network Coordinator
CSMA_SEED_12	equ	2		; Seed Value for CSMA Random Number Generator
CSMA_SEED_11	equ	1
CSMA_SEED_10	equ	0

CSMA_BE		sfr	0x16f		; Transceiver CSMA-CA Back-off Exponent Control Register
MAX_BE3		equ	7		; Maximum Back-off Exponent
MAX_BE2		equ	6
MAX_BE1		equ	5
MAX_BE0		equ	4
MIN_BE3		equ	3		; Minimum Back-off Exponent
MIN_BE2		equ	2
MIN_BE1		equ	1
MIN_BE0		equ	0

MAFCR0		sfr	0x10c		; Multiple Address Filter Configuration Register 0
MAF3EN		equ	3		; Multiple Address Filter 3 Enable
MAF2EN		equ	2		; Multiple Address Filter 2 Enable
MAF1EN		equ	1		; Multiple Address Filter 1 Enable
MAF0EN		equ	0		; Multiple Address Filter 0 Enable

MAFCR1		sfr	0x10d		; Multiple Address Filter Configuration Register 1
AACK_3_SET_PD	equ	7		; Set Data Pending bit for address filter 3
AACK_3_I_AM_COORD equ	6		; Enable PAN Coordinator mode for address filter 3
AACK_2_SET_PD	equ	5		; Set Data Pending bit for address filter 2
AACK_2_I_AM_COORD equ	4		; Enable PAN Coordinator mode for address filter 2
AACK_1_SET_PD	equ	3		; Set Data Pending bit for address filter 1
AACK_1_I_AM_COORD equ	2		; Enable PAN Coordinator mode for address filter 1
AACK_0_SET_PD	equ	1		; Set Data Pending bit for address filter 0
AACK_0_I_AM_COORD equ	0		; Enable PAN Coordinator mode for address filter 0

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
TST_CTRL_DIG_7	equ	7		; Disable Receiver Baseband Frequency Synthesis
TST_CTRL_DIG_6	equ	6		; Disable Receiver Baseband Drift Compensation
TST_CTRL_DIG_5	equ	5		; Enable Switch of Transceiver FIFO
TST_CTRL_DIG_4	equ	4		; Switch Receiver Input Data
TST_CTRL_DIG3	equ	3		; Digital Test Controller Register
TST_CTRL_DIG2	equ	2
TST_CTRL_DIG1	equ	1
TST_CTRL_DIG0	equ	0

TST_RX_LENGTH	sfr	0x17b		; Transceiver Received Frame Length Register

TRXFBST		sfr	0x180		; Start of frame buffer

TRXFBEND	sfr	0x1ff		; End of frame buffer

TRXPR		sfr	0x139		; Transceiver Pin Register
ATBE		equ	3		; Analog Test-bus Enable
TRXTST		equ	2		; Transceiver Test-mode Enable
SLPTR		equ	1		; Multi-purpose Transceiver Control Bit
TRXRST		equ	0		; Force Transceiver Reset

		restore			; re-enable listing

		endif			; __tr24inc
