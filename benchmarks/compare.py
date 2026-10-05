#!/usr/bin/env python3
"""Compare ASL absolute code files, ignoring only the final creator string."""
import argparse
import hashlib
from pathlib import Path
import struct


def code_records(path):
    data = path.read_bytes()
    if data[:2] != b'\x89\x14':
        raise ValueError('Not an ASL code file: {}'.format(path))
    offset = 2
    while offset < len(data):
        kind = data[offset]
        offset += 1
        if kind == 0:
            return data[:offset], data[offset:].decode('ascii')
        if kind == 0x80:
            offset += 4
        elif 1 <= kind <= 0x7f or kind == 0x81:
            # Generic records have CPU, segment, and granularity before the
            # four-byte address and two-byte length; legacy records do not.
            if kind == 0x81:
                offset += 3
            if offset + 6 > len(data):
                raise ValueError('Truncated record: {}'.format(path))
            length = struct.unpack_from('<H', data, offset + 4)[0]
            offset += 6 + length
        else:
            raise ValueError('Unsupported record type 0x{:02x}: {}'.format(kind, path))
        if offset > len(data):
            raise ValueError('Truncated data: {}'.format(path))
    raise ValueError('Missing end record: {}'.format(path))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('objects', type=Path, nargs='+')
    args = parser.parse_args()
    records = []
    for path in args.objects:
        code, creator = code_records(path)
        records.append(code)
        print('{}  {}  ({})'.format(hashlib.sha256(code).hexdigest(), path, creator))
    if any(code != records[0] for code in records[1:]):
        print('MISMATCH: code records differ')
        return 1
    print('MATCH: all code records are identical')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
