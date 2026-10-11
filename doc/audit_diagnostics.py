import collections
import json
import pathlib
import re
import subprocess

root = pathlib.Path(__file__).resolve().parent.parent
interesting = set('InvAddrMode InvReg InvRegPair InvRegList InvOpSize InvShiftArg DistTooBig JmpDistTooBig OverRange UnderRange AddrNotAligned DistIsOdd InAccPage InAccPageErr CurrPCInInAccPageErr NotOnThisAddress InvFuncArgCnt WrongArgCnt AddrModeNotSupported InstructionNotSupported Unpredictable ConfOpSizes'.split())
rows = []
files = sorted(root.glob('*.c'))
for path in files:
    source = path.read_text(encoding='utf-8')
    # Retain offsets and line numbers while excluding comments and strings.
    cleaned = re.sub(r"""/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*'""", lambda m: ''.join('\n' if c == '\n' else ' ' for c in m.group()), source, flags=re.S)
    lines = source.splitlines()
    for match in re.finditer(r'\b(Wr(?:X|Str)?Error(?:Pos)?)\s*\(\s*(ErrNum_\w+)', cleaned):
        code = match.group(2)[7:]
        if code not in interesting:
            continue
        line = source.count('\n', 0, match.start()) + 1
        rows.append(dict(file=path.name, line=line, emitter=match.group(1), diagnostic=match.group(2), context='\n'.join(lines[max(0,line-6):line+3]), status='candidate; requires semantic review'))
inventory = dict(scope='Root C sources; direct calls whose first argument is a selected literal ErrNum. Excludes dynamic/ternary IDs, helpers, and non-C/catalog files. Candidates are not confirmed findings.', files_scanned=len(files), sites=len(rows), counts=dict(collections.Counter(row['diagnostic'] for row in rows)), sites_by_location=rows)
(root/'doc/diagnostic-audit-inventory.json').write_text(json.dumps(inventory, indent=2)+'\n', encoding='utf-8')
cases = [
    ('m68k-an-index', '68000', ' move.w 128(a0,d0.w),d1'),
    ('m68k-pc-index', '68000', 'target equ 4608\n move.w target(pc,d0.w),d1'),
    ('m68k-pc-index-forced-word', '68000', 'target equ 4608\n move.w target.w(pc,d0.w),d1'),
    ('m68k-pc-word-overflow', '68000', 'target equ 1048576\n move.w target.w(pc),d1'),
    ('m68k-zero-bsr', '68000', ' bsr.s *+2'),
    ('m68k-far-branch', '68000', ' bra.s *+200'),
    ('m68k-index-size', '68000', ' move.w (a0,d0.b),d1'),
    ('m68k-index-scale', '68000', ' move.w (a0,d0.w*3),d1'),
    ('m68k-missing-scaling-feature', '68000', ' move.w (a0,d0.w*2),d1'),
    ('m68k-duplicate-displacement', '68000', ' move.w 1(2,a0),d1'),
    ('m68k-shift-count', '68000', ' lsl.w #9,d0'),
    ('m68k-memory-shift-size', '68000', ' lsl.b (a0)'),
    ('ppc-branch-alignment', 'MPC601', ' b *+2'),
    ('ppc-branch-range', 'MPC601', ' b *+33554432'),
    ('hc12-auto-count', '68HC12', ' ldaa 9,x+'),
    ('function-arity', '68000', ' dc.l SIN(1,2)'),
    ('mot-data-range', '68000', ' dc.b -129'),
    ('80504-placement', '80C504', ' org 2046\n ajmp 1792'),
]
logs=[]
for name,cpu,body in cases:
    path=root/'build'/('diagnostic-audit-'+name+'.asm')
    text=' cpu '+cpu+'\n org 4096\n'+body+'\n'
    path.write_text(text, encoding='ascii')
    result=subprocess.run([str(root/'build/asl.exe'), str(path), '-n', '-q'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    logs.append(name+'\n'+text+'exit='+str(result.returncode)+'\n'+result.stdout.decode('utf-8',errors='replace'))
(root/'doc/diagnostic-audit-reproductions.txt').write_text('\n'.join(logs), encoding='utf-8')
print(json.dumps(dict(files=len(files), sites=len(rows), counts=inventory['counts'], probes=len(cases)), indent=2))
