#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Explicit QM 5.3.0 generation; normal firmware builds never invoke QM."""
import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET

CLONE = Path(__file__).resolve().parents[1]
REPO = CLONE.parents[1]
MODEL = CLONE / "model" / "blinky.qm"
GENERATED = CLONE / "generated"
WORK = REPO / "build" / "blinky-qm-codegen"
FILES = ("blinky_ao.hpp", "blinky_ao.cpp", "console_ao.hpp", "console_ao.cpp")
DEFAULT_QM = "/Applications/qm.app/Contents/MacOS/qm"


def project_path(value):
    path = Path(value).expanduser().resolve()
    if not path.is_relative_to(REPO):
        raise ValueError(f"Path must remain inside the project: {path}")
    return path


def generator(value):
    requested = value or os.environ.get("QM_EXECUTABLE") or DEFAULT_QM
    executable = shutil.which(requested)
    if not executable:
        raise ValueError(f"QM executable not found: {requested}; supply --qm PATH")
    result = subprocess.run([executable, "-h"], capture_output=True, text=True,
                            check=True, timeout=30)
    banner = result.stdout + result.stderr
    if not re.search(r"(?m)^qm 5\.3\.0(?:\s|$)", banner):
        raise ValueError("QM 5.3.0 is required; refusing a different generator version")
    return executable


def validate_model():
    # Keep every generated file inside the isolated sandbox. Models are trusted
    # project sources, but output-layout mistakes must not write elsewhere.
    root = ET.parse(MODEL).getroot()
    directories = root.findall(".//directory")
    if len(directories) != 1 or directories[0].get("name") != "../generated":
        raise ValueError("Expected one model output directory: ../generated")
    files = directories[0].findall("file")
    if sorted(item.get("name", "") for item in files) != sorted(FILES):
        raise ValueError("Model output file list differs from the approved generated layout")


def differences(expected, directory):
    result = []
    actual = {str(p.relative_to(directory)) for p in directory.rglob("*")
              if p.is_file() and p.name != ".DS_Store"} if directory.exists() else set()
    for name in sorted(actual - set(expected)):
        result.append(f"UNEXPECTED: {name}")
    for name, content in expected.items():
        path = directory / name
        if path.is_symlink():
            result.append(f"SYMLINK: {name}")
        elif not path.is_file():
            result.append(f"MISSING: {name}")
        elif path.read_bytes() != content:
            result.append(f"STALE: {name}")
    return result


def install(expected):
    GENERATED.mkdir(parents=True, exist_ok=True)
    problems = differences(expected, GENERATED)
    if any(p.startswith(("UNEXPECTED:", "SYMLINK:")) for p in problems):
        raise ValueError("Refusing to overwrite unexpected output:\n" + "\n".join(problems))
    for name, content in expected.items():
        target = GENERATED / name
        if target.exists() and target.read_bytes() == content:
            continue
        # Copy generator bytes verbatim. Atomic replacement also handles QM's
        # read-only output without making tracked files writable by hand.
        with tempfile.NamedTemporaryFile(dir=GENERATED, prefix=f".{name}.",
                                         delete=False) as stream:
            staging = Path(stream.name)
            stream.write(content)
        try:
            staging.chmod(0o444)
            os.replace(staging, target)
        finally:
            staging.unlink(missing_ok=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("regenerate", "check", "diagrams"))
    parser.add_argument("--qm", help="QM 5.3.0 executable (or set QM_EXECUTABLE)")
    parser.add_argument("--generated-dir", help="check this project-local output directory instead")
    parser.add_argument("--output", help="project-local diagram destination")
    args = parser.parse_args()
    if args.generated_dir and args.mode != "check":
        parser.error("--generated-dir is only valid for check")
    if args.output and args.mode != "diagrams":
        parser.error("--output is only valid for diagrams")
    try:
        executable = generator(args.qm)
        validate_model()
        WORK.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(prefix="run-", dir=WORK) as temporary:
            sandbox = Path(temporary)
            (sandbox / "model").mkdir()
            (sandbox / "generated").mkdir()
            shutil.copyfile(MODEL, sandbox / "model" / MODEL.name)
            options = ["-c"] if args.mode != "diagrams" else ["-d", "*.svg", "-d", "*.png"]
            subprocess.run([executable, str(sandbox / "model" / MODEL.name), *options],
                           cwd=sandbox, check=True, timeout=60)
            if args.mode == "diagrams":
                destination = project_path(args.output or REPO / "build" / "blinky-qm-diagrams")
                destination.mkdir(parents=True, exist_ok=True)
                diagrams = [p for p in sandbox.iterdir() if p.suffix in (".svg", ".png")]
                if len(diagrams) != 4:
                    raise ValueError(f"Expected two SVG and two PNG diagrams, found {len(diagrams)}")
                for diagram in diagrams:
                    shutil.copyfile(diagram, destination / diagram.name)
                print(f"Exported both diagrams (SVG/PNG) to {destination}")
            else:
                output = sandbox / "generated"
                actual = {p.name for p in output.iterdir() if p.is_file()}
                if actual != set(FILES):
                    raise ValueError(f"Unexpected generator output set: {sorted(actual)}")
                expected = {name: (output / name).read_bytes() for name in FILES}
                if args.mode == "regenerate":
                    install(expected)
                    print("Generated C++ synchronized with the model (no manual edits).")
                else:
                    directory = project_path(args.generated_dir or GENERATED)
                    problems = differences(expected, directory)
                    if problems:
                        print("\n".join(problems), file=sys.stderr)
                        return 1
                    print("Generated C++ is byte-identical to QM 5.3.0 output.")
        return 0
    except (ValueError, OSError, ET.ParseError, subprocess.SubprocessError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
