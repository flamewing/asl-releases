# Shipped Include Files

<!-- markdownlint-disable MD001 -->
<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->
<!-- markdownlint-disable MD036 -->

The distribution of AS contains a couple of include files. Apart from include files that only refer to a specific processor family (and whose function should be immediately clear to someone who works with this family), there are a few processor-independent files which include useful functions. The functions defined in these files shall be explained briefly in the following sections:

## BITFUNCS.INC

This file defines a couple of bit-oriented functions that might be hardwired for other assemblers. In the case of AS however, they are implemented with the help of user-defined functions:

- _mask(start,bits)_ returns an integer with _bits_ bits set starting at position _start_;
- _invmask(start,bits)_ returns one's complement to _mask()_;
- _cutout(x,start,bits)_ returns _bits_ bits masked out from _x_ starting at position _start_ without shifting them to position 0;
- _hi(x)_ returns the second lowest byte (bits 8..15) of _x_;
- _lo(x)_ returns the lowest byte (bits 8..15) of _x_;
- _hiword(x)_ returns the second lowest word (bits 16..31) of _x_;
- _loword(x)_ returns the lowest word (bits 0..15) of _x_;
- _odd(x)_ returns TRUE if _x_ is odd;
- _even(x)_ returns TRUE if _x_ is even;
- _getbit(x,n)_ extracts bit _n_ out of _x_ and returns it as 0 or 1;
- _shln(x,size,n)_ shifts a word _x_ of length _size_ to the left by _n_ places;
- _shrn(x,size,n)_ shifts a word _x_ of length _size_ to the right by _n_ places;
- _rotln(x,size,n)_ rotates the lowest _size_ bits of an integer _x_ to the left by _n_ places;
- _rotrn(x,size,n)_ rotates the lowest _size_ bits of an integer _x_ to the right by _n_ places;

## CTYPE.INC

This include file is similar to the C include file `ctype.h` which
offers functions to classify characters. All functions deliver either
TRUE or FALSE:

- _isdigit(ch)_ becomes TRUE if _ch_ is a valid decimal digit (0..9);
- _isxdigit(ch)_ becomes TRUE if _ch_ is a valid hexadecimal digit (0..9, A..F, a..f);
- _isupper(ch)_ becomes TRUE if _ch_ is an uppercase letter, excluding special national characters);
- _islower(ch)_ becomes TRUE if _ch_ is a lowercase letter, excluding special national characters);
- _isalpha(ch)_ becomes TRUE if _ch_ is a letter, excluding special national characters;
- _isalnum(ch)_ becomes TRUE if _ch_ is either a letter or a valid decimal digit;
- _isspace(ch)_ becomes TRUE if _ch_ is an 'empty' character (space, form feed, line feed, carriage return, tabulator);
- _isprint(ch)_ becomes TRUE if _ch_ is a printable character, i.e. no control character up to code 31;
- _iscntrl(ch)_ is the opposite to _isprint()_;
- _isgraph(ch)_ becomes TRUE if _ch_ is a printable and visible character;
- _ispunct(ch)_ becomes TRUE if _ch_ is a printable special character (i.e. neither space nor letter nor number);

# Acknowledgments

> _"If I have seen farther than other men, it is because I stood on the shoulders of giants."_
>
> _–Sir Isaac Newton_

<!-- Breaking block quote -->

> _"If I haven't seen farther than other men, it is because I stood in the footsteps of giants."_
>
> _–unknown_

If one decides to rewrite a chapter that has been out of date for two years, it is almost unavoidable that one forgets to mention some of the good ghosts who contributed to the success this project had up to now. The first "thank you" therefore goes to the people whose names I unwillingly forgot in the following enumeration!

The concept of AS as a universal cross assembler came from Bernhard (C.) Zschocke who needed a "student friendly", i.e. free cross assembler for his microprocessor course and talked me into extending an already existing 68000 assembler. The rest is history... The microprocessor course held at RWTH Aachen also always provided the most engaged users (and bug-searchers) of new AS features and therefore contributed a lot to today's quality of AS.

The internet and FTP have proved to be a big help for spreading AS and reporting of bugs. My thanks therefore go to the FTP admins (Bernd Casimir in Stuttgart, Norbert Breidor in Aachen, and Jürgen Meißburger in Jülich). Especially the last one personally engaged a lot to establish a practicable way in Jülich.

As we are just talking about the ZAM: Though Wolfgang E. Nagel is not personally involved into AS, he is at least my boss and always puts at least four eyes on what I am doing. Regarding AS, there seems to be at least one that smiles...

A program like AS cannot be done without appropriate data books and documentation. I received information from an enormous amount of people, ranging from tips up to complete data books. An enumeration follows (as stated before, without guarantee for completeness!):

Ernst Ahlers, Charles Altmann, Marco Awater, Len Bayles, Andreas Bolsch, Rolf Buchholz, Bernd Casimir, Nils Eilers, Gunther Ewald, Michael Haardt, Stephan Hruschka, Peter Kliegelhöfer, Ulf Meinke, Udo Möller, Matthias Paul, Norbert Rosch, Curt J. Sampson, Steffen Schmid, Leonhard Schneider, Ernst Schwab, Michael Schwingen, Oliver Sellke, Christian Stelter, Patrik Strömdahl, Tadashi G. Takaoka, Oliver Thamm, Thorsten Thiele, Leszek Ulman, Rob Warmelink, Andreas Wassatsch, John Weinrich.

...and an ironic "thank you" to Rolf-Dieter-Klein and Tobias Thiel who demonstrated with their ASM68K how one should **not** do it and thereby indirectly gave me the impulse to write something better!

I did not entirely write AS on my own. The DOS version of AS contained the OverXMS routines from Wilbert van Leijen which can move the overlay modules into the extended memory. A really nice library, easy to use without problems!

The TMS320C2x/5x code generators and the file `STDDEF2x.INC` come from Thomas Sailer, ETH Zurich. It's surprising, he only needed one weekend to understand my coding and to implement the new code generator. Either that was a long night shift or I am slowly getting old...the same praise goes to Haruo Asano for providing the MN1610/MN1613 code generator.

<!-- markdownlint-enable MD036 -->
<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
<!-- markdownlint-enable MD001 -->
