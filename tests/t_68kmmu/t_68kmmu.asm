		cpu	68020

		fpu	on
		pmmu	on
		supmode	on

		pflusha
		pflush	#4,#3
		pflush	d2,#3
		pflush	sfc,#3
		pflush	dfc,#3
		pflushs	#4,#3
		pflushs	d2,#3
		pflushs	sfc,#3
		pflushs	dfc,#3
		pflush	#4,#3,(a4)
		pflush	d2,#3,(a4)
		pflush	sfc,#3,(a4)
		pflush	dfc,#3,(a4)
		pflushs	#4,#3,(a4)
		pflushs	d2,#3,(a4)
		pflushs	sfc,#3,(a4)
		pflushs	dfc,#3,(a4)
