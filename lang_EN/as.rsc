/* lang_EN/as.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* String-Definitionen fuer AS - englisch                                    */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           19. 1.1997 Kommandozeilenoption U                               */
/*           21. 1.1997 Warnung nicht bitadressierbare Speicherstelle        */
/*           22. 1.1997 Fehler/Warnungen fuer Stacks                         */
/*            1. 2.1997 Warnung wegen NUL-Zeichen                            */
/*           29. 3.1997 Kommandozeilenoption g                               */
/*           30. 5.1997 Warnung wg. inkorrektem Listing                      */
/*           12. 7.1997 Kommandozeilenoption Y                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Fehlermeldungen */

#ifdef ERRMSG
#define ErrName             " : error "
#define WarnName            " : warning "
#define InLineName          " in line "

#define ErrMsgUselessDisp          "useless displacement 0"
#define ErrMsgShortAddrPossible    "short addressing possible"
#define ErrMsgShortJumpPossible    "short jump possible"
#define ErrMsgNoShareFile          "no sharefile created, SHARED ignored"
#define ErrMsgBigDecFloat          "FPU possibly cannot read this value (> 1E1000)"
#define ErrMsgPrivOrder            "privileged instruction"
#define ErrMsgDistNull             "distance of 0 not allowed for short jump (NOP created instead)"
#define ErrMsgWrongSegment         "symbol out of wrong segment"
#define ErrMsgInAccSegment         "segment not accessible"
#define ErrMsgPhaseErr      	   "change of symbol values forces additional pass"
#define ErrMsgOverlap              "overlapping memory usage"
#define ErrMsgNoCaseHit            "none of the CASE conditions was true"
#define ErrMsgInAccPage            "page might not be addressable"
#define ErrMsgRMustBeEven          "register number must be even"
#define ErrMsgObsolete	           "obsolete instruction, usage discouraged"
#define ErrMsgUnpredictable        "unpredictable execution of this instruction"
#define ErrMsgAlphaNoSense         "localization operator senseless out of a section"
#define ErrMsgSenseless            "senseless instruction"
#define ErrMsgRepassUnknown        "unknown symbol value forces additional pass"
#define ErrMsgAddrNotAligned       "address is not properly aligned"
#define ErrMsgPipeline             "possible pipelining effects"
#define ErrMsgDoubleAdrRegUse      "multiple use of address register in one instruction"
#define ErrMsgNotBitAddressable    "memory location is not bit addressable"
#define ErrMsgStackNotEmpty        "stack is not empty"
#define ErrMsgNULCharacter         "NUL character in string, result is undefined"
#define ErrMsgDoubleDef            "symbol double defined"
#define ErrMsgIOAddrNotAllowed     "I/O-address must not be used here"
#define ErrMsgSymbolUndef          "symbol undefined"
#define ErrMsgInvSymName           "invalid symbol name"
#define ErrMsgInvFormat            "invalid format"
#define ErrMsgUseLessAttr          "useless attribute"
#define ErrMsgTooLongAttr          "attribute may only be one character long"
#define ErrMsgUndefAttr            "undefined attribute"
#define ErrMsgWrongArgCnt          "wrong number of operands"
#define ErrMsgWrongOptCnt          "wrong number of options"
#define ErrMsgOnlyImmAddr          "addressing mode must be immediate"
#define ErrMsgInvOpsize            "invalid operand size"
#define ErrMsgConfOpSizes          "conflicting operand sizes"
#define ErrMsgUndefOpSizes         "undefined operand size"
#define ErrMsgInvOpType            "invalid operand type"
#define ErrMsgTooMuchArgs          "too many arguments"
#define ErrMsgUnknownOpcode        "unknown opcode"
#define ErrMsgBrackErr             "number of opening/closing brackets does not match"
#define ErrMsgDivByZero            "division by 0"
#define ErrMsgUnderRange           "range underflow"
#define ErrMsgOverRange            "range overflow"
#define ErrMsgNotAligned           "address is not properly aligned"
#define ErrMsgDistTooBig           "distance too big"
#define ErrMsgInAccReg             "register not accessible"
#define ErrMsgNoShortAddr          "short addressing not allowed"
#define ErrMsgInvAddrMode          "addressing mode not allowed here"
#define ErrMsgMustBeEven           "number must be even"
#define ErrMsgParInvAddrMode       "addressing mode not allowed in parallel operation"
#define ErrMsgInvParAddrMode       "addressing mode not allowed im parallel operation"
#define ErrMsgUndefCond            "undefined condition"
#define ErrMsgJmpDistTooBig        "jump distance too big"
#define ErrMsgDistIsOdd            "jump distance is odd"
#define ErrMsgInvShiftArg          "invalid argument for shifting"
#define ErrMsgRange18              "operand must be in range 1..8"
#define ErrMsgShiftCntTooBig       "shift amplitude too big"
#define ErrMsgInvRegList           "invalid register list"
#define ErrMsgInvCmpMode           "invalid addressing mode for CMP"
#define ErrMsgInvCPUType           "invalid CPU type"
#define ErrMsgInvCtrlReg           "invalid control register"
#define ErrMsgInvReg               "invalid register"
#define ErrMsgNoSaveFrame          "RESTORE without SAVE"
#define ErrMsgNoRestoreFrame       "missing RESTORE"
#define ErrMsgUnknownMacArg        "unknown macro control instruction"
#define ErrMsgMissEndif            "missing ENDIF/ENDCASE"
#define ErrMsgInvIfConst           "invalid IF-structure"
#define ErrMsgDoubleSection        "section name double defined"
#define ErrMsgInvSection           "unknown section"
#define ErrMsgMissingEndSect       "missing ENDSECTION"
#define ErrMsgWrongEndSect         "wrong ENDSECTION"
#define ErrMsgNotInSection         "ENDSECTION without SECTION"
#define ErrMsgUndefdForward        "unresolved forward declaration"
#define ErrMsgContForward          "conflicting FORWARD <-> PUBLIC-declaration"
#define ErrMsgInvFuncArgCnt        "wrong numbers of function arguments"
#define ErrMsgMissingLTORG         "unresolved literals (missing LTORG)"
#define ErrMsgNotOnThisCPU1        "order not allowed on "
#define ErrMsgNotOnThisCPU2        ""
#define ErrMsgNotOnThisCPU3        "addressing mode not allowed on "
#define ErrMsgInvBitPos            "invalid bit position"
#define ErrMsgOnlyOnOff            "only ON/OFF allowed"
#define ErrMsgStackEmpty           "stack is empty or undefined"
#define ErrMsgShortRead            "unexpected end of file"
#define ErrMsgRomOffs063           "ROM-offset must be in range 0..63"
#define ErrMsgInvFCode             "invalid function code"
#define ErrMsgInvFMask             "invalid function code mask"
#define ErrMsgInvMMUReg            "invalid MMU register"
#define ErrMsgLevel07              "level must be in range 0..7"
#define ErrMsgInvBitMask           "invalid bit mask"
#define ErrMsgInvRegPair           "invalid register pair"
#define ErrMsgOpenMacro            "open macro definition"
#define ErrMsgDoubleMacro          "macro double defined"
#define ErrMsgTooManyMacParams     "more than 10 macro parameters"
#define ErrMsgEXITMOutsideMacro    "EXITM not called from within macro"
#define ErrMsgFirstPassCalc        "expression must be evaluatable in first pass"
#define ErrMsgTooManyNestedIfs     "too many nested IFs"
#define ErrMsgMissingIf            "ELSEIF/ENDIF without IF"
#define ErrMsgRekMacro             "nested / recursive makro call"
#define ErrMsgUnknownFunc          "unknown function"
#define ErrMsgInvFuncArg           "function argument out of definition range"
#define ErrMsgFloatOverflow        "floating point overflow"
#define ErrMsgInvArgPair           "invalid value pair"
#define ErrMsgNotOnThisAddress     "order must not start on this address"
#define ErrMsgNotFromThisAddress   "invalid jump target"
#define ErrMsgTargOnDiffPage       "jump target not on same page"
#define ErrMsgCodeOverflow         "code overflow"
#define ErrMsgMixDBDS              "constants and placeholders cannot be mixed"
#define ErrMsgOnlyInCode           "code may only be generated in code segment"
#define ErrMsgParNotPossible       "parallel construct not possible here"
#define ErrMsgAdrOverflow          "address overflow"
#define ErrMsgInvSegment           "invalid segment"
#define ErrMsgUnknownSegment       "unknown segment"
#define ErrMsgUnknownSegReg        "unknown segment register"
#define ErrMsgInvString            "invalid string"
#define ErrMsgInvRegName           "invalid register name"
#define ErrMsgInvArg               "invalid argument"
#define ErrMsgNoIndir              "indirect mode not allowed"
#define ErrMsgNotInThisSegment     "not allowed in current segment"
#define ErrMsgNotInMaxmode         "not allowed in maximum mode"
#define ErrMsgOnlyInMaxmode        "not allowed in minimum mode"
#define ErrMsgOpeningFile          "error in opening file"
#define ErrMsgListWrError          "error in writing listing"
#define ErrMsgFileReadError        "file read error"
#define ErrMsgFileWriteError       "file write error"
#define ErrMsgIntError             "internal error/warning"
#define ErrMsgHeapOvfl             "heap overflow"
#define ErrMsgStackOvfl            "stack overflow"

#define ErrMsgIsFatal       "fatal error, assembly terminated"

#define ErrMsgOvlyError     "overlay error - program terminated"

#define PrevDefMsg          "previous definition in"
#endif

/****************************************************************************/
/* Strings in Listingkopfzeile */

#define HeadingFileNameLab  " - source file "
#define HeadingPageLab      " - page "

/****************************************************************************/
/* Strings in Listing */

#define ListSymListHead1      "  symbol table (* = unused):"
#define ListSymListHead2      "  ------------------------"
#define ListSymSumMsg         " symbol"
#define ListSymSumsMsg        " symbols"
#define ListUSymSumMsg        " unused symbol"
#define ListUSymSumsMsg       " unused symbols"
#define ListMacListHead1      "  defined macros:"
#define ListMacListHead2      "  ---------------"
#define ListMacSumMsg         " macro"
#define ListMacSumsMsg        " macros"
#define ListFuncListHead1     "  defined functions:"
#define ListFuncListHead2     "  ------------------"
#define ListDefListHead1      "  DEFINEs:"
#define ListDefListHead2      "  --------"
#define ListSegListHead1      "space used in "
#define ListSegListHead2      " :"
#define ListCrossListHead1    "  cross reference list:"
#define ListCrossListHead2    "  ---------------------"
#define ListSectionListHead1  "  sections:"
#define ListSectionListHead2  "  ---------"
#define ListIncludeListHead1  "  nested include files:"
#define ListIncludeListHead2  "  ---------------------"
#define ListCrossSymName      "symbol "
#define ListCrossFileName     "file "

#define ListPlurName          "s"
#define ListHourName          " hour"
#define ListMinuName          " minute"
#define ListSecoName          " second"

/****************************************************************************/
/* Durchsagen... */

#define InfoMessAssembling  "assembling "
#define InfoMessPass        "PASS "
#define InfoMessPass1       "PASS 1                             "
#define InfoMessPass2       "PASS 2                             "
#define InfoMessAssTime     " assembly time"
#define InfoMessAssLine     " line source file"
#define InfoMessAssLines    " lines source file"
#define InfoMessPassCnt     " pass"
#define InfoMessPPassCnt    " passes"
#define InfoMessNoPass      "        additional necessary passes not started due to\n        errors, listing possibly incorrect"
#define InfoMessMacAssLine  " line incl. macro expansions"
#define InfoMessMacAssLines " lines incl. macro expansions"
#define InfoMessWarnCnt     " warning"
#define InfoMessWarnPCnt    "s"
#define InfoMessErrCnt      " error"
#define InfoMessErrPCnt     "s"
#define InfoMessRemainMem   " bytes available memory"
#define InfoMessRemainStack " bytes available stack"

#define InfoMessNFilesFound ": no file(s) to assemble!"

#define InfoMessMacroAss    "macro assembler "
#define InfoMessVar         "version"

#ifdef MAIN
#define InfoMessHead1      "calling convention : "
#define InfoMessHead2      " [options] [file] [options] ..."
#define InfoMessHelpCnt    32
static char *InfoMessHelp[InfoMessHelpCnt]=
		  {"--------------------",
		   "",
		   "options :",
		   "---------",
		   "",
		   "-p : share file formatted for Pascal  -c : share file formatted for C",
		   "-a : share file formatted for AS",
                   "-o <name> : change name of code file",
                   "-q,  -quiet : silent compilation",
                   "-alias <new>=<old> : define processor alias",
                   "-l : listing to console               -L : listing to file",
		   "-i <path>[;path]... : list of paths for include files",
		   "-D <symbol>[,symbol]... : predefine symbols",
		   "-E <name> : target file for error list,",
		   "            !0..!4 for standard handles",
		   "            default is <srcname>.LOG",
		   "-r : generate messages if repassing necessary",
                   "-Y : branch error suppression (see manual)",
		   "-w : suppress warnings                +G : suppress code generation",
		   "-s : generate section list            -t : enable/disable parts of listing",
		   "-u : generate usage list              -C : generate cross reference list",
                   "-I : generate include nesting list",
                   "-g : write debug info",
		   "-A : compact symbol table",
                   "-U : case-sensitive operation",
		   "-x : extended error messages          -n : add error #s to error messages",
		   "-P : write macro processor output     -M : extract macro definitions",
		   "-h : use lower case in hexadecimal output",
		   "",
                   "source file specification may contain wildcards",
		   "",
                   "implemented processors :"};

#define KeyWaitMsg           "--- <ENTER> to go on ---"

#define ErrMsgInvParam       "invalid option: "
#define ErrMsgInvEnvParam    "invalid environment option: "

#define InvMsgSource         "Quelldatei?"

#endif
