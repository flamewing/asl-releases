@echo off
md %1
set binfiles=asl.exe plist.exe alink.exe pbind.exe p2hex.exe p2bin.exe
for %%i in (%binfiles%) do tdstrip %%i
for %%i in (%binfiles%) do copy %%i %1
ren %1\asl.exe as.exe
set binfiles=
copy *.msg %1 

md %2
for %%i in (include\*.inc) do copy %%i %2
for %%i in (%2\*.inc) do unumlaut %%i
md %2\avr
for %%i in (include\avr\*.inc) do copy %%i %2\avr
for %%i in (%2\avr\*.inc) do unumlaut %%i
md %2\s12z
for %%i in (include\s12z\*.inc) do copy %%i %2\s12z
for %%i in (%2\s12z\*.inc) do unumlaut %%i
md %2\s12z\vh
for %%i in (include\s12z\vh\*.inc) do copy %%i %2\s12z\vh
for %%i in (%2\s12z\vh\*.inc) do unumlaut %%i
md %2\s12z\vc
for %%i in (include\s12z\vc\*.inc) do copy %%i %2\s12z\vc
for %%i in (%2\s12z\vc\*.inc) do unumlaut %%i
md %2\s12z\vca
for %%i in (include\s12z\vca\*.inc) do copy %%i %2\s12z\vca
for %%i in (%2\s12z\vca\*.inc) do unumlaut %%i
md %2\coldfire
for %%i in (include\coldfire\*.inc) do copy %%i %2\coldfire
for %%i in (%2\coldfire\*.inc) do unumlaut %%i
md %2\stm8
for %%i in (include\stm8\*.inc) do copy %%i %2\stm8
for %%i in (%2\stm8\*.inc) do unumlaut %%i
md %2\stm8\stm8s
for %%i in (include\stm8\stm8s\*.inc) do copy %%i %2\stm8\stm8s
for %%i in (%2\stm8\stm8s\*.inc) do unumlaut %%i
md %2\stm8\stm8l
for %%i in (include\stm8\stm8l\*.inc) do copy %%i %2\stm8\stm8l
for %%i in (%2\stm8\stm8l\*.inc) do unumlaut %%i
md %2\stm8\stm8af
for %%i in (include\stm8\stm8af\*.inc) do copy %%i %2\stm8\stm8af
for %%i in (%2\stm8\stm8af\*.inc) do unumlaut %%i

md %3
for %%i in (man\*.1) do copy %%i %3

md %4
for %%i in (*.msg) do copy %%i %1

md %5
set docdirs=DE EN
for %%i in (%docdirs%) do copy doc_%%i\as.doc %5\as_%%i.doc
for %%i in (%docdirs%) do copy doc_%%i\as.tex %5\as_%%i.tex
for %%i in (%docdirs%) do copy doc_%%i\as.htm %5\as_%%i.htm
copy doc_DE\taborg*.tex %5
copy doc_DE\ps*.tex %5
copy COPYING %5
