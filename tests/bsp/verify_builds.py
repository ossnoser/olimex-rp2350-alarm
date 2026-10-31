#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Inspect the two existing Ninja builds; never flash or run firmware."""
from pathlib import Path
import shlex
import subprocess

REPO = Path(__file__).resolve().parents[2]


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def main():
    for board in ("pico2", "pico2_w"):
        build = REPO / "build" / board
        cache = {}
        for line in (build / "CMakeCache.txt").read_text().splitlines():
            if "=" in line and not line.startswith(("#", "//")):
                key, value = line.split("=", 1)
                cache[key.split(":", 1)[0]] = value
        require(cache.get("PICO_BOARD") == board, f"Wrong board in {build}")
        require(cache.get("PICO_PLATFORM") == "rp2350-arm-s", f"Wrong platform in {build}")
        expected_name = "Pico 2 Blinky" if board == "pico2" else "Pico 2 W Blinky"
        other_name = "Pico 2 W Blinky" if board == "pico2" else "Pico 2 Blinky"
        selected_led = f"/bsp/{board}/led.cpp"
        other_led = "/bsp/pico2_w/led.cpp" if board == "pico2" else "/bsp/pico2/led.cpp"
        for example, target in (("blinky", "qpcpp-blinky"), ("blinky-qm", "qpcpp-blinky-qm")):
            commands = subprocess.check_output(
                ["ninja", "-C", str(build), "-t", "commands", target], text=True)
            sources = []
            for command in commands.splitlines():
                arguments = shlex.split(command)
                if "-c" in arguments:
                    sources.append(arguments[arguments.index("-c") + 1])
            require(any(p.endswith(selected_led) for p in sources), f"Missing {selected_led}")
            require(not any(p.endswith(other_led) for p in sources), f"Unexpected {other_led}")
            require(any(p.endswith("/bsp/common/usb_descriptors.cpp") for p in sources),
                    "Shared USB descriptors were not built")
            wireless = [p for p in sources if "cyw43" in p.lower()]
            require(bool(wireless) == (board == "pico2_w"),
                    f"Unexpected CYW43 dependency selection: {board}/{example}")
            require(not any("/pico_stdio_usb/" in p or "/pico_stdio_uart/" in p for p in sources),
                    "USB/UART stdio must remain disabled")
            artifact = build / "examples" / example / target
            elf = artifact.with_suffix(".elf")
            uf2 = artifact.with_suffix(".uf2")
            require(elf.stat().st_size > 0 and uf2.stat().st_size > 0, "Missing firmware artifact")
            symbols = subprocess.check_output(["arm-none-eabi-nm", "-C", str(elf)], text=True)
            require("BSP::Board::usbProductName()" in symbols, "Missing selected USB identity")
            require(("cyw43_arch_init" in symbols) == (board == "pico2_w"),
                    f"Wrong linked LED implementation: {board}/{example}")
            data = elf.read_bytes()
            require(expected_name.encode() + b"\0" in data, "USB product name missing")
            require(other_name.encode() + b"\0" not in data, "Wrong board's USB product name linked")
            print(f"PASS: {board}/{example}: correct LED/dependencies, USB identity, ELF and UF2")


if __name__ == "__main__":
    main()
