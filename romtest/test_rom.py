import argparse
import pathlib
import subprocess
import sys
import typing
from dataclasses import dataclass
from pathlib import Path

NES_SCRREN_ROWS = 30
NES_SCRREN_COLS = 32


@dataclass
class Test:
    rom: str
    pass_pattern: str
    failing: bool = False
    cycles: int = 100000000


TESTS = [
    # PPU tests
    Test(
        rom="oam_read/oam_read.nes",
        pass_pattern="Passed",
        cycles=10000000,
    ),
    Test(
        rom="vbl_nmi_timing/1.frame_basics.nes", pass_pattern="PASSED", cycles=100000000
    ),
    Test(
        rom="vbl_nmi_timing/2.vbl_timing.nes",
        pass_pattern="PASSED",
        cycles=100000000,
        failing=True,
    ),  # FAILED #8
    Test(
        rom="vbl_nmi_timing/3.even_odd_frames.nes",
        pass_pattern="PASSED",
        cycles=100000000,
        failing=True,
    ),  # FAILED #3
    Test(
        rom="vbl_nmi_timing/4.vbl_clear_timing.nes",
        pass_pattern="PASSED",
        cycles=100000000,
    ),
    Test(
        rom="vbl_nmi_timing/5.nmi_suppression.nes",
        pass_pattern="PASSED",
        cycles=100000000,
        failing=True,
    ),  # FAILED #3
    Test(
        rom="vbl_nmi_timing/6.nmi_disable.nes",
        pass_pattern="PASSED",
        cycles=100000000,
        failing=True,
    ),  # FAILED #2
    Test(
        rom="vbl_nmi_timing/7.nmi_timing.nes",
        pass_pattern="PASSED",
        cycles=100000000,
        failing=True,
    ),  # FAILED #2
    Test(
        rom="ppu_open_bus/ppu_open_bus.nes",
        pass_pattern="PASSED",
        cycles=100000000,
        failing=True,
    ),  # FAILED #2
    # CPU tests
    Test(
        rom="cpu_dummy_reads/cpu_dummy_reads.nes", pass_pattern="PASSED", failing=True
    ),  # Unsupported mapper 3
    Test(
        rom="cpu_dummy_writes/cpu_dummy_writes_ppumem.nes",
        pass_pattern="PASSED",
        failing=True,
        cycles=100000000,
    ),  # Failed #9
    Test(
        rom="cpu_dummy_writes/cpu_dummy_writes_oam.nes",
        pass_pattern="0ASSED",  # Tile indexes are not mapped to ascii for all characters.
        cycles=120000000,
    ),
    Test(
        rom="cpu_exec_space/test_cpu_exec_space_ppuio.nes",
        pass_pattern="PASSED",
        cycles=10000000,
        failing=True,
    ),  # Failed #3, ppu open bus
    Test(
        rom="cpu_timing_test6/cpu_timing_test.nes",
        pass_pattern="PASSED",
        cycles=1000000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/01-basics.nes",
        pass_pattern="Passed",
        cycles=10000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/02-implied.nes",
        pass_pattern="Passed",
        cycles=100000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/03-immediate.nes",
        pass_pattern="Passed",
        cycles=100000000,
        failing=True,
    ),  # Missing opcode 0x89
    Test(
        rom="instr_test-v5/rom_singles/04-zero_page.nes",
        pass_pattern="Passed",
        cycles=100000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/05-zp_xy.nes",
        pass_pattern="Passed",
        cycles=100000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/06-absolute.nes",
        pass_pattern="Passed",
        cycles=100000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/07-abs_xy.nes",
        pass_pattern="Passed",
        cycles=100000000,
        failing=True,
    ),  # Missing opcode 0x9c
    Test(
        rom="instr_test-v5/rom_singles/08-ind_x.nes",
        pass_pattern="Passed",
        cycles=100000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/09-ind_y.nes",
        pass_pattern="Passed",
        cycles=100000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/10-branches.nes",
        pass_pattern="Passed",
        cycles=100000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/11-stack.nes",
        pass_pattern="Passed",
        cycles=100000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/12-jmp_jsr.nes",
        pass_pattern="Passed",
        cycles=10000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/13-rts.nes",
        pass_pattern="Passed",
        cycles=10000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/14-rti.nes",
        pass_pattern="Passed",
        cycles=10000000,
    ),
    Test(
        rom="instr_test-v5/rom_singles/15-brk.nes",
        pass_pattern="Passed",
        cycles=10000000,
        failing=True,
    ),
    Test(
        rom="instr_test-v5/rom_singles/16-special.nes",
        pass_pattern="Passed",
        cycles=10000000,
    ),
]


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--rom-repo-path", type=pathlib.Path, default="nes-test-roms")
    parser.add_argument("--romtest-bin", type=pathlib.Path, required=True)
    parser.add_argument(
        "--filter", help="If set, only run tests matching this argument"
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Print more debug data if set"
    )
    args = parser.parse_args()
    return args


def run_test(
    romtest_bin: Path, rom_repo_path: Path, test: Test
) -> typing.Dict[typing.Tuple[int, int], int]:
    rom = rom_repo_path / test.rom
    print(f'Running rom: "{rom}" with binary: "{romtest_bin}"')
    output = subprocess.run(
        [romtest_bin, rom, str(test.cycles)], capture_output=True, text=True
    )
    if output.returncode != 0:
        print(
            f"ERROR! Could not run rom: {rom}\nstdout: {output.stdout}\nstderr: {output.stderr}"
        )
        return {}
    grid: typing.Dict[typing.Tuple[int, int], int] = {}
    for row, line in enumerate(output.stdout.splitlines()[2:]):
        for col, tile_index_hex in enumerate(line.split(",")):
            if tile_index_hex:
                grid[(row, col)] = int(tile_index_hex, 16)
    return grid


def is_ascii_in_output(grid: typing.Dict, ascii_text: str, verbose: bool) -> bool:
    for row in range(0, NES_SCRREN_ROWS):
        line_ascii = "".join(chr(grid[(row, col)]) for col in range(0, NES_SCRREN_COLS))
        if verbose and line_ascii.strip():
            print(line_ascii)
        if ascii_text in line_ascii:
            return True
    return False


def main():
    args = parse_args()
    success = True

    if args.filter:
        print(f"Running only tests matching: {args.filter}")

    for test in TESTS:
        if args.filter and args.filter not in test.rom:
            continue
        grid = run_test(
            romtest_bin=args.romtest_bin, rom_repo_path=args.rom_repo_path, test=test
        )
        if grid:
            test_passed = is_ascii_in_output(
                grid=grid, ascii_text=test.pass_pattern, verbose=args.verbose
            )
        else:
            test_passed = False

        if test_passed:
            print("Success!")
        else:
            print("Fail!")

        if test.failing and not test_passed:
            print("WARNING! Test did not pass but is marked as failing")
        elif test.failing and test_passed:
            print("ERROR! Test passed but is marked as failing")
        else:
            success &= test_passed

    if success:
        print("All tests passed!")
    else:
        print("Some tests failed")
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
