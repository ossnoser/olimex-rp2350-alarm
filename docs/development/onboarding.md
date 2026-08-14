# Developer onboarding guide

This guide lists the practical steps required to become operational on this firmware project using a macOS Intel development PC.

## 1. Open the project

```bash
cd /Users/ossnoser/Documents/prj/olimex-rp2350-alarm
code .
```

## 2. Install base tools

Git is already installed on the development PC.

Install the remaining base tools, for example with Homebrew:

```bash
brew install cmake ninja python
```

Verify the tools:

```bash
git --version
cmake --version
ninja --version
python3 --version
```

## 3. Install Visual Studio Code extensions

Install these recommended VS Code extensions:

- Microsoft C/C++;
- CMake Tools;
- Raspberry Pi Pico extension, if confirmed suitable for the selected RP2350/Pico2 workflow.

Optional for source-level debugging:

- Cortex-Debug.

## 4. Install ARM GNU embedded toolchain

Install an ARM bare-metal toolchain that provides:

- `arm-none-eabi-gcc`;
- `arm-none-eabi-g++`;
- `arm-none-eabi-gdb`;
- `arm-none-eabi-objcopy`;
- `arm-none-eabi-size`.

Verify the compiler tools:

```bash
arm-none-eabi-gcc --version
arm-none-eabi-g++ --version
arm-none-eabi-gdb --version
```

## 5. Install or clone Raspberry Pi Pico SDK

Use the shared Pico SDK directory outside this project repository:

```bash
mkdir -p /Users/ossnoser/Documents/prj/pico
cd /Users/ossnoser/Documents/prj/pico
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git submodule update --init
```

Configure the SDK path for the current shell by sourcing the project environment script from the project root:

```bash
cd /Users/ossnoser/Documents/prj/olimex-rp2350-alarm
source env.sh
```

Verify:

```bash
echo "$PICO_SDK_PATH"
```

Expected output:

```text
/Users/ossnoser/Documents/prj/pico/pico-sdk
```

## 6. Install picotool

`picotool` is recommended for working with RP2350/Pico2 firmware images and devices in BOOTSEL mode.

Possible installation method:

```bash
brew install picotool
```

Verify:

```bash
picotool version
```

## 7. Prepare qpcpp for consultation

Do not integrate `qpcpp` into this repository until a dedicated approved task defines the integration method.

For consultation, clone it outside the project repository:

```bash
mkdir -p ~/Developer/quantum-leaps
cd ~/Developer/quantum-leaps
git clone https://github.com/QuantumLeaps/qpcpp --recurse-submodules --depth 1
```

## 8. Decide whether SWD debugging is needed

SWD debugging is not required for the first project setup.

If source-level debugging is required later, prepare:

- OpenOCD compatible with RP2350;
- an SWD probe, such as Raspberry Pi Debug Probe, PicoProbe, CMSIS-DAP, or J-Link;
- VS Code Cortex-Debug extension;
- `arm-none-eabi-gdb`, usually provided by the ARM GNU embedded toolchain.

## 9. Recommended next project task

The recommended next task is:

```text
TASK-0009 - Development environment setup verification
```

Suggested scope:

- verify installed tools;
- select official tool versions;
- document final installation commands;
- create a setup checklist or verification script;
- then proceed to create the first CMake/Pico SDK firmware project skeleton.
