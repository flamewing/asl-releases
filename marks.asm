ConstBUF32 equ 65.12
ConstFL900 equ 33.52
ConstMIC51 equ 359.52


	read	"BUF32, Pass1 [sec]: ", buf32_1
	read	"FL900, Pass1 [sec]: ", fl900_1
	read	"MIC51, Pass1 [sec]: ", mic51_1
	read	"BUF32, Pass2 [sec]: ", buf32_2
	read	"FL900, Pass2 [sec]: ", fl900_2
	read	"MIC51, Pass2 [sec]: ", mic51_2
	read	"BUF32, Pass3 [sec]: ", buf32_3
	read	"FL900, Pass3 [sec]: ", fl900_3
	read	"MIC51, Pass3 [sec]: ", mic51_3

buf32	equ	(buf32_1+buf32_2+buf32_3)/3.0
	message	"--> BUF32= \{BUF32}"
fl900	equ	(fl900_1+fl900_2+fl900_3)/3.0
	message	"--> Fl900= \{FL900}"
mic51	equ	(mic51_1+mic51_2+mic51_3)/3.0
	message	"--> MIC51= \{MIC51}"

marks	equ	((ConstBUF32/buf32)+(ConstFL900/fl900)+(ConstMIC51/mic51))/3
	message	"--> Marks= \{MARKS}"

	read	"Clk [MHz]: ", ClkFreq
rel	equ	marks/ClkFreq
	message	"Rel=\{REL}"

