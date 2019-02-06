{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei AS.RSC - enth„lt Stringdefinitionen fr AS (englisch)        *}
{* 									    *}
{* Historie : 20.1.1993 Grundsteinlegung                                    *}
{*   			SPEEDUP-Symbol					    *}
{*            21.1.1993 Fehlermeldungen                                     *}
{*            12.6.1993 E/A-Fehlermeldungen                                 *}
{*            4. 9.1993 Lokalfehlermeldungen                                *}
{*            5.10.1993 Umstellung auf Englisch begonnen                    *}
{*           30.10.1993 Optionen G, M                                       *}
{*            1.11.1993 Erkennung Programmname                              *}
{*           14.11.1992 Warnung veraltete Anweisung                         *}
{*           15.11.1993 Option E                                            *}
{*           16. 1.1994 Meldungen sinnlos, falsche Ausrichtung              *}
{*           23. 1.1993 Meldungen wg. Repass                                *}
{*           15. 4.1994 zus„tzliche Fehlerstrings                           *}
{*           24. 4.1994 Fehlermeldung ungltiges Format                     *}
{*	      3. 5.1994 Fehlermeldung doppeltes Makro                       *}
{*           24. 7.1994 Meldung freier Stack                                *}
{*           20. 8.1994 Kopfzeilen Define-Liste                             *}
{*           21. 8.1994 Meldung undefiniertes Attribut                      *}
{*           25. 9.1994 restliche Meldungen TMS320C3x                       *}
{*           16.10.1994 Melung unaufgel”ste Literale                        *}
{*           10.12.1994 Kommandozeilenoption o                              *}
{*           25.12.1994 Kommandozeilenoption q                              *}
{*           15. 6.1996 Meldung ungltige Optionszahl                       *}
{*            2. 1.1996 Meldung vorherige Definition                        *}
{*           26. 1.1996 Kopfzeilen Includeliste                             *}
{*           24. 3.1996 Parameterausgabe vervollst„ndigt                    *}
{*            6. 9.1996 EXITM-Fehlermeldung                                 *}
{*           14.10.1996 BINCLUDE-Fehlermeldung                              *}
{*           19. 1.1997 Kommandozeilenoption U                              *}
{*           21. 1.1997 Warnung nicht bitadressierbare Speicherstelle       *}
{*           22. 1.1997 Fehler/Warnungen fr Stacks                         *}
{*           25. 1.1997 Fehler fr Bitmaskenfunktion                        *}
{*            1. 2.1997 Warnung wegen NUL-Zeichen                           *}
{*           10. 2.1997 Warnung wegen Seitenberschreitung                  *}
{*           29. 3.1997 Kommandozeilenoption g                              *}
{*           29. 5.1997 Warnung wg. inkorrektem Listing                     *}
{*           12. 7.1997 Kommandozeilenoption Y                              *}
{*           10. 8.1997 Meldungen fr Strukturen                            *}
{*           23. 8.1997 Atmel-Debug-Format                                  *}
{*            7. 9.1997 Warnung Bereichsberschreitung                      *}
{*           24. 9.1997 Kopfzeile Registerdefinitionsliste                  *}
{*           19.10.1997 Warnung neg. DUP-Anzahl                             *}
{*           11. 1.1998 C6x-Fehlermeldungen                                 *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ wegnehmen, falls reines Pascal gewnscht }

{$DEFINE SPEEDUP}


{****************************************************************************}
{ Fehlermeldungen }

CONST
   ErrName                  = ' : error ';
   WarnName                 = ' : warning ';
   InLineName               = ' in line ';

   ErrMsgUselessDisp        = 'useless displacement 0';
   ErrMsgShortAddrPossible  = 'short addressing possible';
   ErrMsgShortJumpPossible  = 'short jump possible';
   ErrMsgNoShareFile        = 'no sharefile created, SHARED ignored';
   ErrMsgBigDecFloat        = 'FPU possibly cannot read this value (>=1E1000)';
   ErrMsgPrivOrder          = 'privileged instruction';
   ErrMsgDistNull           = 'distance of 0 not allowed for short jump (NOP created instead)';
   ErrMsgWrongSegment       = 'symbol out of wrong segment';
   ErrMsgInAccSegment       = 'segment not accessible';
   ErrMsgPhaseErr      	    = 'change of symbol values forces additional pass';
   ErrMsgOverlap            = 'overlapping memory usage';
   ErrMsgNoCaseHit          = 'none of the CASE conditions was true';
   ErrMsgInAccPage          = 'page might not be addressable';
   ErrMsgRMustBeEven        = 'register number must be even';
   ErrMsgObsolete	    = 'obsolete instruction, usage discouraged';
   ErrMsgUnpredictable      = 'unpredictable execution of this instruction';
   ErrMsgAlphaNoSense       = 'localization operator senseless out of a section';
   ErrMsgSenseless          = 'senseless instruction';
   ErrMsgRepassUnknown      = 'unknown symbol value forces additional pass';
   ErrMsgAddrNotAligned     = 'address is not properly aligned';
   ErrMsgPipeline           = 'possible pipelining effects';
   ErrMsgDoubleAdrRegUse    = 'multiple use of address register in one instruction';
   ErrMsgNotBitAddressable  = 'memory location not bit addressable';
   ErrMsgStackNotEmpty      = 'stack is not empty';
   ErrMsgPageCrossing       = 'instruction crosses page boundary';
   ErrMsgWOverRange         = 'range overflow';
   ErrMsgNegDUP             = 'negative argument to DUP';

   ErrMsgDoubleDef          = 'symbol double defined';
   ErrMsgNULCharacter       = 'NUL character in string, result is undefined';
   ErrMsgIOAddrNotAllowed   = 'I/O-address must not be used here';
   ErrMsgSymbolUndef        = 'symbol undefined';
   ErrMsgInvSymName         = 'invalid symbol name';
   ErrMsgInvFormat          = 'invalid format';
   ErrMsgUselessAttr        = 'useless attribute';
   ErrMsgTooLongAttr        = 'attribute may only be one character long';
   ErrMsgUndefAttr          = 'undefined attribute';
   ErrMsgWrongArgCnt        = 'wrong number of operands';
   ErrMsgWrongOptCnt        = 'wrong number of options';
   ErrMsgOnlyImmAddr        = 'addressing mode must be immediate';
   ErrMsgInvOpsize          = 'invalid operand size';
   ErrMsgConfOpSizes        = 'conflicting operand sizes';
   ErrMsgUndefOpSizes       = 'undefined operand size';
   ErrMsgInvOpType          = 'invalid operand type';
   ErrMsgTooMuchArgs        = 'too many arguments';
   ErrMsgUnknownOpcode      = 'unknown opcode';
   ErrMsgBrackErr           = 'number of opening/closing brackets does not match';
   ErrMsgDivByZero          = 'division by 0';
   ErrMsgUnderRange         = 'range underflow';
   ErrMsgOverRange          = 'range overflow';
   ErrMsgNotAligned         = 'address is not properly aligned';
   ErrMsgDistTooBig         = 'distance too big';
   ErrMsgInAccReg           = 'register not accessible';
   ErrMsgNoShortAddr        = 'short addressing not allowed';
   ErrMsgInvAddrMode        = 'addressing mode not allowed here';
   ErrMsgMustBeEven         = 'number must be even';
   ErrMsgParInvAddrMode     = 'addressing mode not allowed in parallel operation';
   ErrMsgInvParAddrMode     = 'addressing mode not allowed im parallel operation';
   ErrMsgUndefCond          = 'undefined condition';
   ErrMsgJmpDistTooBig      = 'jump distance too big';
   ErrMsgDistIsOdd          = 'jump distance is odd';
   ErrMsgInvShiftArg        = 'invalid argument for shifting';
   ErrMsgRange18            = 'operand must be in range 1..8';
   ErrMsgShiftCntTooBig     = 'shift amplitude too big';
   ErrMsgInvRegList         = 'invalid register list';
   ErrMsgInvCmpMode         = 'invalid addressing mode for CMP';
   ErrMsgInvCPUType         = 'invalid CPU type';
   ErrMsgInvCtrlReg         = 'invalid control register';
   ErrMsgInvReg             = 'invalid register';
   ErrMsgNoSaveFrame        = 'RESTORE without SAVE';
   ErrMsgNoRestoreFrame     = 'missing RESTORE';
   ErrMsgUnknownMacArg      = 'unknown macro control instruction';
   ErrMsgMissEndif          = 'missing ENDIF/ENDCASE';
   ErrMsgInvIfConst         = 'invalid IF-structure';
   ErrMsgDoubleSection      = 'section name double defined';
   ErrMsgInvSection         = 'unknown section';
   ErrMsgMissingEndSect     = 'missing ENDSECTION';
   ErrMsgWrongEndSect       = 'wrong ENDSECTION';
   ErrMsgNotInSection       = 'ENDSECTION without SECTION';
   ErrMsgUndefdForward      = 'unresolved forward declaration';
   ErrMsgContForward        = 'conflicting FORWARD <-> PUBLIC-declaration';
   ErrMsgInvFuncArgCnt      = 'wrong numbers of function arguments';
   ErrMsgMissingLTORG       = 'unresolved literals (missing LTORG)';
   ErrMsgNotOnThisCPU1      = 'order not allowed on ';
   ErrMsgNotOnThisCPU2      = '';
   ErrMsgNotOnThisCPU3      = 'addressing mode not allowed on ';
   ErrMsgInvBitPos          = 'invalid bit position';
   ErrMsgOnlyOnOff          = 'only ON/OFF allowed';
   ErrMsgStackEmpty         = 'stack is empty or undefined';
   ErrMsgNotOneBit          = 'not exactly one bit is set';
   ErrMsgMissingStruct      = 'ENDSTRUCT without STRUCT';
   ErrMsgOpenStruct         = 'open structure definition';
   ErrMsgWrongStruct        = 'wrong ENDSTRUCT';
   ErrMsgPhaseDisallowed    = 'phase definition not allowed in structure definition';
   ErrMsgInvStructDir       = 'invalid STRUCT directive';
   ErrMsgRomOffs063         = 'ROM-offset must be in range 0..63';
   ErrMsgShortRead          = 'unexpected end of file';
   ErrMsgInvFCode           = 'invalid function code';
   ErrMsgInvFMask           = 'invalid function code mask';
   ErrMsgInvMMUReg          = 'invalid MMU register';
   ErrMsgLevel07            = 'level must be in range 0..7';
   ErrMsgInvBitMask         = 'invalid bit mask';
   ErrMsgInvRegPair         = 'invalid register pair';
   ErrMsgOpenMacro          = 'open macro definition';
   ErrMsgDoubleMacro        = 'macro double defined';
   ErrMsgTooManyMacParams   = 'more than 10 macro parameters';
   ErrMsgEXITMOutsideMacro  = 'EXITM not called from within macro';
   ErrMsgFirstPassCalc      = 'expression must be evaluatable in first pass';
   ErrMsgTooManyNestedIfs   = 'too many nested IFs';
   ErrMsgMissingIf          = 'ELSEIF/ENDIF without IF';
   ErrMsgRekMacro           = 'nested / recursive makro call';
   ErrMsgUnknownFunc        = 'unknown function';
   ErrMsgInvFuncArg         = 'function argument out of definition range';
   ErrMsgFloatOverflow      = 'floating point overflow';
   ErrMsgInvArgPair         = 'invalid value pair';
   ErrMsgNotOnThisAddress   = 'order must not start on this address';
   ErrMsgNotFromThisAddress = 'invalid jump target';
   ErrMsgTargOnDiffPage     = 'jump target not on same page';
   ErrMsgCodeOverflow       = 'code overflow';
   ErrMsgMixDBDS            = 'constants and placeholders cannot be mixed';
   ErrMsgNotInStruct        = 'code must not be generated in structure definition';
   ErrMsgParNotPossible     = 'parallel construct not possible here';
   ErrMsgAdrOverflow        = 'address overflow';
   ErrMsgInvSegment         = 'invalid segment';
   ErrMsgUnknownSegment     = 'unknown segment';
   ErrMsgUnknownSegReg      = 'unknown segment register';
   ErrMsgInvString          = 'invalid string';
   ErrMsgInvRegName         = 'invalid register name';
   ErrMsgInvArg             = 'invalid argument';
   ErrMsgNoIndir            = 'indirect mode not allowed';
   ErrMsgNotInThisSegment   = 'not allowed in current segment';
   ErrMsgNotInMaxmode       = 'not allowed in maximum mode';
   ErrMsgOnlyInMaxmode      = 'not allowed in minimum mode';
   ErrMsgPacketNotAligned   = 'instruction packet crosses 8 word boundary';
   ErrMsgDoubleUnitUsed     = 'functional unit used double';
   ErrMsgLongReads          = 'more than one long read operand';
   ErrMsgLongWrites         = 'more than one long write operand';
   ErrMsgLongMem            = 'long read operand with memory access';
   ErrMsgTooManyReads       = 'too many read accesses to register';
   ErrMsgWriteConflict      = 'write conflict on register';
   ErrMsgOpeningFile        = 'error in opening file';
   ErrMsgListWrError        = 'error in writing listing';
   ErrMsgFileReadError      = 'file read error';
   ErrMsgFileWriteError     = 'file write error';
   ErrMsgIntError           = 'internal error/warning';
   ErrMsgHeapOvfl           = 'heap overflow';
   ErrMsgStackOvfl          = 'stack overflow';

   ErrMsgIsFatal            = 'fatal error, assembly terminated';

   ErrMsgOvlyError          = 'overlay error - program terminated';

   PrevDefMsg               = 'previous definition in';

{$IFDEF DPMI}
   ErrMsgInvSwapSize        = 'swap file size not correctly specified - program terminated';
   ErrMsgSwapTooBig         = 'insufficient space for swap file - program terminated';
{$ENDIF}

{****************************************************************************}
{ Strings in Listingkopfzeile }

   HeadingFileNameLab = ' - source file ';
   HeadingPageLab     = ' - page ';

{****************************************************************************}
{ Strings in Listing }

   ListSymListHead1     = '  symbol table (*=unused):';
   ListSymListHead2     = '  ------------------------';
   ListSymSumMsg        = ' symbol';
   ListSymSumsMsg       = ' symbols';
   ListUSymSumMsg       = ' unused symbol';
   ListUSymSumsMsg      = ' unused symbols';
   ListRegDefListHead1  = '  register definitions (*=unused):';
   ListRegDefListHead2  = '  ------------------------------------';
   ListRegDefSumMsg     = ' definition';
   ListRegDefSumsMsg    = ' definitions';
   ListRegDefUSumMsg    = ' unused definition';
   ListRegDefUSumsMsg   = ' unused definitions';
   ListMacListHead1     = '  defined macros:';
   ListMacListHead2     = '  ---------------';
   ListMacSumMsg        = ' macro';
   ListMacSumsMsg       = ' macros';
   ListFuncListHead1    = '  defined functions:';
   ListFuncListHead2    = '  ------------------';
   ListDefListHead1     = '  DEFINEs:';
   ListDefListHead2     = '  --------';
   ListSegListHead1     = 'space used in ';
   ListSegListHead2     = ' :';
   ListCrossListHead1   = '  cross reference list:';
   ListCrossListHead2   = '  ---------------------';
   ListSectionListHead1 = '  sections:';
   ListSectionListHead2 = '  ---------';
   ListIncludeListHead1 = '  nested include files:';
   ListIncludeListHead2 = '  ---------------------';
   ListCrossSymName     = 'symbol ';
   ListCrossFileName    = 'file ';

   ListPlurName         = 's';
   ListHourName         = ' hour';
   ListMinuName         = ' minute';
   ListSecoName         = ' second';

{****************************************************************************}
{ Durchsagen... }

   InfoMessAssembling = 'assembling ';
   InfoMessPass       = 'PASS ';
   InfoMessPass1      = 'PASS 1                             ';
   InfoMessPass2      = 'PASS 2                             ';
   InfoMessAssTime    = ' assembly time';
   InfoMessAssLine    = ' line source file';
   InfoMessAssLines   = ' lines source file';
   InfoMessPassCnt    = ' pass';
   InfoMessPPassCnt   = ' passes';
   InfoMessNoPass     = '        additional necessary passes not started due to'+#13+#10+
                        '        errors, listing possibly incorrect';
   InfoMessMacAssLine = ' line incl. macro expansions';
   InfoMessMacAssLines= ' lines incl. macro expansions';
   InfoMessWarnCnt    = ' warning';
   InfoMessWarnPCnt   = 's';
   InfoMessErrCnt     = ' error';
   InfoMessErrPCnt    = 's';
   InfoMessRemainMem  = ' KByte available memory';
   InfoMessRemainStack= ' Byte available stack';

   InfoMessNFilesFound= ': no file(s) to assemble!';

   InfoMessMacroAss   = 'macro assembler ';
   InfoMessVar        = 'version';

{$IFDEF MAIN}
   InfoMessHead1      = 'calling convention : ';
   InfoMessHead2      = ' [options] [file] [options] ...';
   InfoMessHelpCnt    = 32;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
		  ('--------------------',
		   '',
		   'options :',
		   '---------',
		   '',
		   '-p : share file formatted for Pascal  -c : share file formatted for C',
		   '-a : share file formatted for AS',
                   '-o <name> : change name of code file',
                   '-q,  -quiet : silent compilation',
                   '-alias <new>=<old> : define prozessor alias',
                   '-l : listing to console               -L : listing to file',
		   '-i <path>[;path]... : list of paths for include files',
		   '-D <symbol>[,symbol]... : predefine symbols',
		   '-E <name> : target file for error list,',
		   '            !0..!4 for standard handles',
		   '            default is <srcname>.LOG',
		   '-r : generate messages if repassing necessary',
                   '-Y : branch error suppression (see manual)',
		   '-w : suppress warnings                +G : suppress code generation',
		   '-s : generate section list            -t : enable/disable parts of listing',
		   '-u : generate usage list              -C : generate cross reference list',
                   '-I : generate include nesting list',
                   '-g : write debug info [MAP/ATMEL]',
                   '-A : compact symbol table',
                   '-U : case-sensitive operation',
                   '-x : extended error messages          -n : add error #s to error messages',
		   '-P : write macro processor output     -M : extract macro definitions',
		   '-h : use lower case in hexadecimal output',
		   '',
                   'source file specification may contain wildcards',
		   '',
                   'implemented processors :');

   KeyWaitMsg         = '--- <ENTER> to go on ---';

   ErrMsgInvParam     = 'invalid option: ';
   ErrMsgInvEnvParam  = 'invalid environment option: ';

   InvMsgSource       = 'Quelldatei?';

{$ENDIF}
