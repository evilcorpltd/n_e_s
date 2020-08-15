import argparse
import difflib
import pathlib
import subprocess
import sys
import typing
import re
from pathlib import Path


THIS_FILE = pathlib.Path(__file__).absolute().parent
REMOVE_REGEX = re.compile('PPU:\s*[-0-9]+,\s*[0-9]+')


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--nestest-log",
                        type=pathlib.Path,
                        default="nestest.log")
    parser.add_argument("--nestest-rom",
                        type=pathlib.Path,
                        default="nestest.nes")
    parser.add_argument("--nestest-bin",
                        type=pathlib.Path)
    parser.add_argument("--min-matching-lines",
                        type=int)
    args = parser.parse_args()
    return args


def run_nestest(nestest_bin: Path, nestest_rom: Path) -> typing.List[str]:
    print(f'Running rom: "{nestest_rom}" with binary: "{nestest_bin}"')
    output = subprocess.run([nestest_bin, nestest_rom],
                            capture_output=True,
                            text=True)
    lines = []
    for l in output.stdout.splitlines():
        if 'Bad instruction' in l:
            lines.append(l.strip())
            break
        line = REMOVE_REGEX.sub('', l).strip()
        lines.append(line)
    return lines


def load_log(nestest_log: Path) -> typing.List[str]:
    print(f'Opening file: "{nestest_log}"')
    lines = []
    with nestest_log.open('r') as f:
        for l in f:
            line = REMOVE_REGEX.sub('', l).strip()
            lines.append(line)
    return lines


def main():
    args = parse_args()

    actual_data = run_nestest(args.nestest_bin, args.nestest_rom)
    expected_data = load_log(args.nestest_log)

    success = True
    for line, (actual, expected) in enumerate(zip(actual_data, expected_data)):
        if actual != expected:
            success = False
            d = difflib.Differ()
            diff = d.compare([actual], [expected])
            print(f'Missmatch detected on line: {line}')
            print('\n'.join(diff))

    if args.min_matching_lines:
        if line < args.min_matching_lines:
            print(f'Expected at least: {args.min_matching_lines} successful lines')
            sys.exit(1)
        else:
            sys.exit(0)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
