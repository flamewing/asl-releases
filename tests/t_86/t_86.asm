	cpu	8086

	in	ax,dx
	in	al,dx
	in	ax,50h
	in	al,50h
	out	dx,ax
	out	dx,al
	out	50h,ax
	out	50h,al
