# Project environment script

The project root contains a short environment initialization script:

```text
env.sh
```

## Purpose

The script configures environment variables needed while working on this firmware project.

Currently it sets:

```bash
export PICO_SDK_PATH=/Users/ossnoser/Documents/prj/pico/pico-sdk
```

This path points to the shared Raspberry Pi Pico SDK checkout intended to be used by this and future Pico/RP2350 projects.

## Usage

Environment variables must be applied to the current shell. For this reason, source the script from the project root:

```bash
source env.sh
```

or:

```bash
. env.sh
```

Do not rely on direct execution:

```bash
./env.sh
```

Direct execution runs the script in a child process and does not update the current shell environment.

## Verification

After sourcing the script, verify:

```bash
echo "$PICO_SDK_PATH"
```

Expected output:

```text
/Users/ossnoser/Documents/prj/pico/pico-sdk
```

## Future use

The script can later host additional project-specific environment setup, such as:

- toolchain paths;
- build configuration variables;
- SDK version checks;
- helper aliases or shell functions, if approved by a future task.
