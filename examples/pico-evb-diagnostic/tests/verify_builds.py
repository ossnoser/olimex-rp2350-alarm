#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Inspect configured/build artifacts only. Never flash or open a USB port."""
from pathlib import Path
import subprocess
import re

REPO = Path(__file__).resolve().parents[3]
TARGET = 'qpcpp-pico-evb-diagnostic'
for board in ('pico2', 'pico2_w'):
    build = REPO / 'build' / board
    targets = subprocess.check_output(['ninja', '-C', str(build), '-t', 'targets', 'all'], text=True)
    assert (TARGET in targets) == (board == 'pico2'), board
    examples = [('blinky', 'qpcpp-blinky'), ('blinky-qm', 'qpcpp-blinky-qm')]
    if board == 'pico2':
        examples.append(('pico-evb-diagnostic', TARGET))
    for example, target in examples:
        diagnostic = target == TARGET
        commands = subprocess.check_output(['ninja', '-C', str(build), '-t', 'commands', target], text=True)
        assert ('/bsp/carriers/pico_evb_rev_a/carrier.cpp' in commands) == diagnostic
        elf = build / 'examples' / example / (target + '.elf')
        assert elf.with_suffix('.uf2').stat().st_size > 0
        symbols = subprocess.check_output(['arm-none-eabi-nm', '-C', str(elf)], text=True)
        for function in ('Carrier::initRelaysOff()', 'Carrier::forceRelaysOff()', 'Carrier::sample()',
                         'Carrier::activateRelay(unsigned int)', 'Carrier::deactivateRelay(unsigned int)', 'Carrier::commandedRelays()'):
            assert (function in symbols) == diagnostic, (board, example, function)
        assert (b'Pico 2 EVB Diagnostic\0' in elf.read_bytes()) == diagnostic
        config = re.search(r'^([0-9a-f]+)\s+r\s+\(anonymous namespace\)::CONFIG$', symbols, re.M)
        assert config, 'Missing configuration descriptor'
        address = int(config[1], 16)
        dump = subprocess.check_output(['arm-none-eabi-objdump', '-s',
            f'--start-address={address}', f'--stop-address={address + 9}', str(elf)], text=True)
        header = b''
        for line in dump.splitlines():
            if re.match(rf'^\s*{address:x}\s', line):
                header = b''.join(bytes.fromhex(word) for word in line.split()[1:]
                    if re.fullmatch(r'(?:[0-9a-f]{2}){1,4}', word))
        assert len(header) == 9 and header[8] == 125, 'Expected 250 mA USB declaration'
        if diagnostic:
            assert 'BlinkyAO::' in symbols and 'DiagnosticAO::' in symbols and 'RelayTestHsm::' in symbols
            assert '/examples/blinky-qm/' not in commands and '/examples/blinky/' not in commands
        print(f'PASS: {board}/{example}: carrier source/symbol/identity isolation')
