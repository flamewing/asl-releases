#!/usr/bin/env python3
"""Alternate uninstrumented ASL baseline/candidate runs on one input snapshot."""
import argparse
import json
import os
from pathlib import Path
import platform
import shutil
import statistics
import subprocess
import time

from run import sha256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True, type=Path)
    parser.add_argument('--candidate', required=True, type=Path)
    parser.add_argument('--repos', required=True, type=Path, help='Prepared repository copies')
    parser.add_argument('--out', required=True, type=Path)
    parser.add_argument('--manifest', type=Path, default=Path(__file__).with_name('workloads.json'),
                        help='Optional local workload definitions')
    parser.add_argument('--runs', type=int, default=9)
    parser.add_argument('--workloads', nargs='+', default=['s1', 's2', 'sk'])
    parser.add_argument('--timeout', type=float, default=300)
    args = parser.parse_args()
    if args.runs < 1 or args.timeout <= 0:
        parser.error('runs and timeout must be positive')
    binaries = {'baseline': args.baseline.resolve(), 'candidate': args.candidate.resolve()}
    for binary in binaries.values():
        if not binary.is_file():
            parser.error('Executable does not exist: {}'.format(binary))
    definitions = json.loads(args.manifest.read_text())
    if any(name not in definitions for name in args.workloads):
        parser.error('Unknown workload')
    output = args.out.resolve()
    output.mkdir(parents=True, exist_ok=False)
    env = os.environ.copy()
    env['LC_ALL'] = 'C'
    report = {'platform': platform.platform(), 'runs_per_binary': args.runs,
              'warmups_per_binary': 1, 'listing': False,
              'executables': {label: {'path': str(path), 'sha256': sha256(path)}
                              for label, path in binaries.items()}, 'workloads': {}}
    for name in args.workloads:
        definition = definitions[name]
        directory = output / definition['repository']
        shutil.copytree(str(args.repos.resolve() / definition['repository']), str(directory),
                        ignore=shutil.ignore_patterns('.git'))
        arguments = ['-xx', '-n', '-q', '-A', '-U', '-i', '.']
        arguments += definition['arguments'] + [definition['source']]
        object_file = directory / Path(definition['source']).with_suffix('.p')
        expected = None
        item = {'arguments': arguments, 'source': str(args.repos.resolve()),
                'baseline_seconds': [], 'candidate_seconds': [], 'order': []}
        report['workloads'][name] = item
        for iteration in range(-1, args.runs):
            order = ['baseline', 'candidate'] if iteration % 2 == 0 else ['candidate', 'baseline']
            for label in order:
                if object_file.exists():
                    object_file.unlink()
                start = time.perf_counter()
                process = subprocess.run([str(binaries[label])] + arguments,
                                         cwd=str(directory), env=env, timeout=args.timeout,
                                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                elapsed = time.perf_counter() - start
                (output / '{}-{}-{}.log'.format(name, label, iteration)).write_bytes(process.stdout)
                if process.returncode or not object_file.is_file():
                    raise RuntimeError('{} {} failed; inspect logs'.format(name, label))
                digest = sha256(object_file)
                if expected is None:
                    expected = digest
                elif digest != expected:
                    raise RuntimeError('{} {} output differs from baseline'.format(name, label))
                if iteration >= 0:
                    item[label + '_seconds'].append(elapsed)
            if iteration >= 0:
                item['order'].append(order)
        baseline = statistics.median(item['baseline_seconds'])
        candidate = statistics.median(item['candidate_seconds'])
        item.update(object_sha256=expected, baseline_median_seconds=baseline,
                    candidate_median_seconds=candidate,
                    median_speedup=baseline / candidate,
                    median_time_reduction_percent=100 * (1 - candidate / baseline),
                    paired_speedups=[b / c for b, c in zip(item['baseline_seconds'],
                                                         item['candidate_seconds'])],
                    status='ok')
        print('{}: baseline {:.3f}s, candidate {:.3f}s, time reduction {:.1f}%'.format(
            name, baseline, candidate, item['median_time_reduction_percent']), flush=True)
        (output / 'results.json').write_text(json.dumps(report, indent=2) + '\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
