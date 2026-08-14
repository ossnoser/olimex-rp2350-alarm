# TASK-0001 - Initial project structure

## Status

Approved / completed.

## Objective

Create the initial structure for the embedded project targeting the Olimex board based on RP2350-PICO2, including directories for:

- technical documentation;
- design decisions;
- configuration files;
- operating rules for AI tools;
- skills and specific instructions for AI agents;
- sequential task tracking.

## Planned changes

Create the following elements:

```text
olimex-rp2350-alarm/
├── TASKS.md
├── docs/
│   ├── README.md
│   ├── architecture/
│   ├── decisions/
│   ├── hardware/
│   ├── requirements/
│   └── tasks/
│       └── TASK-0001-project-structure.md
├── config/
│   └── README.md
├── ai/
│   ├── README.md
│   ├── rules/
│   │   └── project-rules.md
│   └── skills/
│       └── README.md
└── .gitignore
```

## Main files

### `TASKS.md`

Sequential index of project tasks.

### `docs/tasks/TASK-0001-project-structure.md`

Document for this task.

### `ai/rules/project-rules.md`

Operating rules for AI tools, including the rule:

> No project file may be modified without a documented and approved task.

### `docs/decisions/`

Directory for architectural and design decisions.

### `docs/requirements/`

Directory for functional and non-functional requirements.

### `docs/hardware/`

Directory for Olimex RP2350-PICO2 board documentation and hardware connections.

## Completion criteria

The task is complete when:

- the directory structure has been created;
- `TASKS.md` contains a reference to `TASK-0001`;
- the initial AI rules are present;
- no files outside the project directory have been modified.

## Result

Completed.
