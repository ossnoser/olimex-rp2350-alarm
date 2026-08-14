# TASK-0009 - Project environment script

## Status

Approved / completed.

## Objective

Create a short project environment script that can be sourced when working in this repository.

The script will configure environment variables needed by the firmware development workflow and can host future project initialization settings.

## Planned changes

Create:

- `env.sh`
- `docs/development/environment.md`
- `docs/tasks/TASK-0009-project-environment-script.md`

Update:

- `TASKS.md`
- `docs/development/README.md`
- `docs/development/onboarding.md`

## Script behavior

The script sets:

```bash
export PICO_SDK_PATH=/Users/ossnoser/Documents/prj/pico/pico-sdk
```

The script must be sourced, not executed directly, so the environment variable affects the current shell.

## Completion criteria

The task is complete when:

- `TASKS.md` lists `TASK-0009`;
- `env.sh` exists in the project root;
- development documentation explains how to use `env.sh`;
- no files outside the project directory have been modified.

## Result

Completed.
