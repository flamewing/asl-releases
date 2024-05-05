# Frequently Asked Questions

<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->

## **EDITOR's NOTE:** This is Alfred Arnold's FAQ section, mostly unedited, preserved for historical reasons

In this chapter, I tried to collect some questions that arise very often together with their answers. Answers to the problems presented in this chapter might also be found at other places in this manual, but one maybe does not find them immediately...

Q:
I am fed up with DOS. Are there versions of AS for other operating systems ?

A:
Apart from the protected mode version that offers more memory when working under DOS, ports exist for OS/2 and Unix systems like Linux (currently in test phase). Versions that help operating system manufacturers located in Redmond to become even richer are currently not planned. I will gladly make the sources of AS available for someone else who wants to become active in this direction. The C variant is probably the best way to start a port into this direction. He should however not expect support from me that goes beyond the sources themselves...

Q:
Is a support of the XYZ processor planned for AS?

A:
New processors are appearing all the time and I am trying to keep pace by extending AS. The stack on my desk labeled "undone" however never goes below the 4 inch watermark... Wishes coming from users of course play an important role in the decision which candidates will be done first. The internet and the rising amount of documentation published in electronic form make the acquisition of data books easier than it used to be, but it always becomes difficult when more exotic or older architectures are wanted. If the processor family in question is not in the list of families that are planned (see chapter 1), adding a data book to a request will have a highly positive influence. Borrowing books is also fine.

Q:
Having a free assembler is really fine, but I now also had use for a disassembler...and a debugger...a simulator would also really be cool!

A:
AS is a project I work on in leisure time, the time I have when I do not have to care of how to make my living. AS already takes a significant portion of that time, and sometimes I make a time-out to use my soldering iron, enjoy a Tangerine Dream CD, watch TV, or simply to fulfill some basic human needs... I once started to write the concept of a disassembler that was designed to create source code that can be assembled and that automatically separates code and data areas. I quickly stopped this project again when I realized that the remaining time simply did not suffice. I prefer to work on one good program than to struggle for half a dozen of mediocre apps. Regarded that way, the answer to the question is unfortunately "no"...

Q:
The screen output of AS is messed up with strange characters, e.g. arrows and brackets. Why?

A:
AS will by default use some ANSI control sequences for screen control. These sequences will appear unfiltered on your screen if you did not install an ANSI driver. Either install an ANSI driver or use the DOS command `SET USEANSI=N` to turn the sequences off.

Q:
AS suddenly terminates with a stack overflow error while assembling my program. Did my program become to large?

A:
Yes and No. Your program's symbol table has grown a bit unsymmetrically what lead to high recursion depths while accessing the table. Errors of this type especially happen in the 16-bit-OS/2 version of AS which has a very limited stack area. Restart AS with the `-A` command line switch. If this does not help, too complex formula expression are also a possible cause of stack overflows. In such a case, try to split the formula into intermediate steps.

Q:
It seems that AS does not assemble my program up to the end. It worked however with an older version of AS (1.39).

A:
Newer versions of AS no longer ignore the `END` statement; they actually terminate assembly when an `END` is encountered. Especially older include files made by some users tended to contain an `END` statement at their end. Simply remove the superfluous `END` statements.

Q:
I made an assembly listing of my program because I had some more complicated assembly errors in my program. Upon closer investigation of the listing, I found that some branches do not point to the desired target but instead to themselves!

A:
This effect happens in case of forward jumps in the first pass. The formula parser does not yet have the target address in its symbol table, and as it is a completely independent module, it has to think of a value that even does not hurt relative branches with short displacement lengths. This is the current program counter itself...in the second pass, the correct values would have appeared, but the second pass did not happen due to errors in the first one. Correct the other errors first so that AS gets into the second pass, and the listing should look more meaningful again.

Q:
Assembly of my program works perfectly, however I get an empty file when I try to convert it with P2HEX or P2BIN.

A:
You probably did not set the address filter correctly. By default, the filter is disabled, i.e. all data is copied to the HEX or binary file. It is however possible to create an empty file if a manually set range does not fit to the addresses used by your program.

Q:
I cannot enter the dollar character when using P2BIN or P2HEX under Unix. The automatic address range setting does not work, instead I get strange error messages.

A:
Unix shells use the dollar character for expansion of shell variables. If you want to pass a dollar character to an application, prefix it with a backslash (`\`). In the special case of the address range specification for P2HEX and P2BIN, you may also use `0x` instead of the dollar character, which removes this problem completely.

Q:
I use AS on a Linux system, the loader program for my target system however runs on a Windows machine. To simplify things, both systems access the same network drive. Unfortunately, the Windows side refuses to read the hex files created by the Linux side :-(

A:
Windows and Linux systems use slightly different formats for text files (hex files are a sort of text files). Windows terminates every line with the characters CR (carriage return) and LF (linefeed), however Linux only uses the linefeed. It depends on the Windows program's 'goodwill' whether it will accept text files in the Linux format or not. If not, it is possible to transfer the files via FTP in ASCII mode instead of a network drive. Alternatively, the hex files can be converted to the Windows format. For example, the program _unix2dos_ can be used to do this, or a small script under Linux:

```sh
    awk '{print $0"\r"}' test.hex >test_cr.hex
```

<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
