# TASK-0017 - QM macOS Intel compatibility investigation

## Status

Approved research / completed.

## Objective

Identify a newer QM release usable on macOS Intel before planning the graphical-model clone of Blinky. Keep QP/C++ pinned at v8.1.5.

## Approval and scope

The user approved downloading and inspecting official macOS QM packages without installing them or replacing the installed QM v5.1.3. This document records the approved research task; it does not authorize installing/running downloaded tools, changing the framework, implementing the clone, flashing, or committing.

## Planned work

- Review official release/platform documentation.
- Download candidate packages only under ignored `build/qm-inspection/`.
- Mount packages read-only at project-local mount points if needed; inspect executable architecture with `file`/`lipo` without launching them, then detach.
- Inspect deployment-target metadata and bundled libraries where useful.
- Record package sources, versions, architectures, and remaining compatibility checks.
- Update `TASKS.md` with this research task and its outcome.

## Completion criteria

- The recommended Intel candidate is supported by executable-header evidence rather than an ambiguous macOS package filename.
- Separate host architecture compatibility from generated-code compatibility with QP/C++ v8.1.5; do not claim an unperformed generation/build test.
- No installation, external-file modifications, dependency/source changes, or unapproved commit.

## Initial findings

The latest stable upstream release is v7.0.4; its README advertises macOS ARM64. Older releases include macOS DMG packages, but their architecture needs binary inspection. The installed tool is QM v5.1.3 at `/Applications/qm.app`.

Sources:

- <https://github.com/QuantumLeaps/qm/releases>
- <https://github.com/QuantumLeaps/qm/tree/v7.0.4>
- <https://www.state-machine.com/qm/history.html>

## Results

Inspected all official stable macOS GUI release packages newer than v5.3.0 in descending version order. Read-only mounting and `lipo -archs` show:

| QM release | GUI executable architecture |
|---|---|
| 7.0.4 | arm64 |
| 7.0.3 | arm64 |
| 7.0.2 | arm64 |
| 7.0.1 | arm64 |
| 7.0.0 | arm64 |
| 6.2.3 | arm64 |
| 6.2.2 | arm64 |
| 6.2.0 | arm64 |
| 6.1.2 | arm64 |
| 5.3.0 | x86_64 |

Therefore **QM v5.3.0 is the newest Intel-compatible GUI package among the official stable GitHub macOS releases inspected**. The corresponding standalone QMC v5.3.0 package is also x86_64. All 19 Mach-O binaries in the GUI image and all 16 in the QMC image were checked; every inspected binary includes x86_64. `otool -l` reports executable deployment target macOS 10.7, but this header alone is not proof of operation on the current host.

Official candidate packages:

- <https://github.com/QuantumLeaps/qm/releases/download/v5.3.0/qm_5.3.0-macos.dmg>
- <https://github.com/QuantumLeaps/qm/releases/download/v5.3.0/qmc_5.3.0-macos.dmg>

Computed SHA-256 values:

- GUI: `307b224f737ae704e9a46b6150a97938748c2084fdd9bbaffc7c6e98e94a86c7`
- QMC: `3a09155b4e68400a9d0e47b5d7a3107d75a06402d0909d59336cfc2e89b92b01`

These are local integrity records, not independently published upstream checksums. For v7.0.4/v7.0.3/v7.0.2, downloaded digests matched the SHA-256 digest fields supplied by the GitHub release API.

All images were detached after inspection. Downloads remain under ignored `build/qm-inspection/`; nothing was installed, no downloaded executable was launched, and `/Applications/qm.app` was not changed. QP/C++ remains pinned at v8.1.5.

Recommendation: propose QM/QMC v5.3.0 as the Intel toolchain for the planned `examples/blinky-qm/` clone. Upstream history states QM v5.3.0 requires QP 7.3.0 or newer, but that is not sufficient to prove generated-code compatibility with QP/C++ v8.1.5. A separately approved tool-execution/model-generation/build check is still needed before implementation. Tool selection, installation, clone implementation, and commits remain subject to approval.
