	cpu	68000

	; single line with very long comment - should be truncated upon reading

	org	0	; this is a very long line wefeilurfuewvbdfwvwmhgedvwmhtdvwhdtwmhdtwdenhtgwdfrgwdfgwrdcngdrscdwm jdzwgeurg2 jezdgwdezgwr364rjdfgjdwejtzdfwjzre34jzehtdfwejtdwjzerfwjztfasdgvyxcmycvsmdfwejtdfwzetr7i34ti2wjkdgweszgfwejfg3i76r4t34ifjhsvfshvfshefgewurftge4r6t4i37gfmjbvfmsdvhcsmdfvgwefgw67herf7g34cd7wegfwe7wuefgrufzgegerjfgejwfgerfgjtzwfdjzswtewtzjtw 346r

	; again, a 'normal' line

	nop

	; line split over several lines

	move.l \
	d0, \
	d1

	; line that becomes too long by continuation, immediately followed by a normal one

	moveq	#-1,d0	; fugekrzfgwudzwjfuwjzftwfjdtwfdhwdfhw2fd2df2jetfhetf2edf2jztdf23 \
		  	  dfbvebfrgejkzfbkjzfdjbwerfwefzwjfgejwhrftwjedfjtfdjwtdfwjtdfwejtdfwedfwjte \
			  fvbkezrfgjweztgwjmhcvwnmcthwvfenctwevndwvdnwtfwnmhtcvwnhedvwjetdvfwtjdvfwt \
			  jfbvejz,rfbejzfemfzbs,cbas,djcfzbekfzgwebrfkzgjhtwvedfwnmgedvwdetvwtedvwtew
	nop
