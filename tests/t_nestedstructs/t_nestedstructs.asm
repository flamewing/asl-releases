	cpu	6809
	page	0

	dottedstructs	on

commin	struct
command  byt ?
 	 union
disk	  struct
slot	   byt ?
size	   adr ?
type	   byt ?
gameid	   byt ?, ?, ?, ?, ?, ?, ?, ?
g2	   byt ?, ?, ?, ?, ?, ?, ?, ?
disk	  endstruct
drive	  struct
letter	   byt ?
name	   byt [10]?
drive 	  endstruct
	 endunion
commin	endstruct

	org	0

	phase	256
a	commin
	dephase

	adr	commin.len
	adr	commin.disk.len
	adr	commin.drive.len

	adr	a.command
	adr	a.disk.slot
	adr	a.disk.size
	adr	a.disk.type
	adr	a.disk.gameid
	adr	a.disk.g2
	adr	a.drive.letter
	adr	a.drive.name

