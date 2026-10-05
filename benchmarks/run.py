#!/usr/bin/env python3
"""Time ASL alone on isolated copies of real disassemblies (Python 3.6+)."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import statistics
import subprocess
import time


def sha256(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(chunk)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--asl', required=True, type=Path)
    parser.add_argument('--repos', required=True, type=Path)
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--manifest', type=Path, default=Path(__file__).with_name('workloads.json'),
                        help='Optional local workload definitions')
    parser.add_argument('--workloads', nargs='+', default=['s1', 's2', 'sk'])
    parser.add_argument('--runs', type=int, default=5)
    parser.add_argument('--warmups', type=int, default=1)
    parser.add_argument('--gprof', type=Path, help='Collect gmon files and generate reports; requires GPROF build')
    parser.add_argument('--listing', action='store_true', help='Include -L as in repository build scripts')
    parser.add_argument('--prepare', action='store_true', help='Generate DAC assets with repository Lua helpers before timing')
    parser.add_argument('--lua', type=Path, help='Lua 5.3+; Windows defaults to the bundled repository Lua')
    parser.add_argument('--timeout', type=float, default=300)
    args = parser.parse_args()
    if args.runs < 1 or args.warmups < 0 or args.timeout <= 0:
        parser.error('runs/timeout must be positive; warmups must be nonnegative')
    executable = args.asl.resolve()
    if not executable.is_file():
        parser.error('ASL executable does not exist: {}'.format(executable))
    definitions = json.loads(args.manifest.read_text())
    for name in args.workloads:
        if name not in definitions:
            parser.error('Unknown workload: {}'.format(name))
    output = args.out.resolve()
    # A fresh directory prevents stale outputs/profiles and records a fresh input snapshot.
    output.mkdir(parents=True, exist_ok=False)
    report = {'platform': platform.platform(), 'python': platform.python_version(),
              'executable': str(executable), 'executable_sha256': sha256(executable),
              'instrumented': bool(args.gprof), 'listing': args.listing,
              'warmups': args.warmups, 'runs': args.runs, 'workloads': {}}
    env = os.environ.copy()
    env['LC_ALL'] = 'C'
    for name in args.workloads:
        definition = definitions[name]
        directory = output / definition['repository']
        source = args.repos.resolve() / definition['repository']
        shutil.copytree(str(source), str(directory), ignore=shutil.ignore_patterns('.git'))
        if args.prepare:
            lua = args.lua.resolve() if args.lua else (
                directory / 'build_tools' / 'lua' / 'lua.exe' if os.name == 'nt'
                else Path(shutil.which('lua') or 'lua'))
            script = ('local c = require "build_tools.lua.common"; '
                      'c.convert_pcm_files_in_directory({}); '
                      'c.convert_dpcm_files_in_directory({})').format(
                          json.dumps(definition['pcm']), json.dumps(definition['dpcm']))
            if definition.get('music'):
                # Reuse the repository's exact music generation logic, stopping
                # before its ROM build. No compression or conversion is timed.
                build_script = (directory / 'build.lua').read_text()
                marker = '-- Compress music data and generate music-related assembler input.'
                if marker not in build_script:
                    raise RuntimeError('Sonic 2 preparation marker is missing from build.lua')
                script = build_script.split(marker, 1)[0] + '\ngenerate_music_data()\n' + script
            prepare_script = directory / 'benchmark-prepare.lua'
            prepare_script.write_text(script)
            with (output / (name + '-prepare.log')).open('wb') as stream:
                subprocess.run([str(lua), str(prepare_script)], cwd=str(directory),
                               stdout=stream, stderr=subprocess.STDOUT,
                               timeout=args.timeout, check=True)
        command = [str(executable), '-xx', '-n', '-q', '-A', '-U', '-i', '.']
        if args.listing:
            command.append('-L')
        command += definition['arguments'] + [definition['source']]
        object_file = directory / Path(definition['source']).with_suffix('.p')
        profiles, samples, hashes = [], [], []
        item = {'command': command, 'source': str(source), 'seconds': samples,
                'object_sha256': hashes, 'status': 'running'}
        report['workloads'][name] = item
        for iteration in range(args.warmups + args.runs):
            if object_file.exists():
                object_file.unlink()
            profile = directory / 'gmon.out'
            if profile.exists():
                profile.unlink()
            started = time.perf_counter()
            try:
                process = subprocess.run(command, cwd=str(directory), env=env,
                                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                         timeout=args.timeout)
                elapsed = time.perf_counter() - started
                log = process.stdout
                code = process.returncode
            except subprocess.TimeoutExpired as error:
                elapsed = time.perf_counter() - started
                log, code = error.stdout or b'', 'timeout'
            (output / '{}-{}.log'.format(name, iteration)).write_bytes(log)
            if code != 0 or not object_file.is_file():
                item.update(status='failed', returncode=code)
                print('{}: FAILED; see {}'.format(name, output / '{}-{}.log'.format(name, iteration)), flush=True)
                break
            if iteration >= args.warmups:
                samples.append(elapsed)
                hashes.append(sha256(object_file))
                if args.gprof:
                    if not profile.is_file():
                        raise RuntimeError('No gmon.out produced; use an ASL_PROFILE=GPROF build')
                    saved = output / '{}-{}.gmon'.format(name, iteration - args.warmups)
                    profile.rename(saved)
                    profiles.append(str(saved))
        if len(samples) == args.runs:
            if len(set(hashes)) != 1:
                raise RuntimeError('{} produced inconsistent objects'.format(name))
            item.update(status='ok', median_seconds=statistics.median(samples),
                        min_seconds=min(samples), max_seconds=max(samples))
            print('{}: median {:.3f}s (min {:.3f}, max {:.3f}), object {}'.format(
                name, item['median_seconds'], min(samples), max(samples), hashes[0]), flush=True)
            if args.gprof:
                with (output / (name + '-gprof.txt')).open('wb') as stream:
                    subprocess.run([str(args.gprof.resolve()), '-b', str(executable)] + profiles,
                                   stdout=stream, check=True)
                if not re.search(r'^\s*\d+\.\d+\s+\d+\.\d+\s+\d+\.\d+',
                                 (output / (name + '-gprof.txt')).read_text(), re.MULTILINE):
                    raise RuntimeError('Empty gprof report for {}; check symbol addresses and sampling'.format(name))
        (output / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    return 0 if all(item['status'] == 'ok' for item in report['workloads'].values()) else 1


if __name__ == '__main__':
    raise SystemExit(main())
