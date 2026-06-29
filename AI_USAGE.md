# AI Usage Disclosure — virtual_routr

This document describes how AI tools (primarily **Cursor** with an embedded coding assistant, and **ChatGPT**-style models for design discussion) were used while building this project.

All generated code and documentation was **manually reviewed, built, tested, debugged, and understood** before being accepted into the repository.

---

## Summary

AI assistance was used as a development accelerator — not as an unattended code generator. The human developer retained responsibility for architecture decisions, correctness, integration, and final acceptance of every change.

| Activity | Primary tool | Human role |
|----------|--------------|------------|
| Architecture & module design | Cursor / ChatGPT | Reviewed, simplified scope, approved structure |
| Project scaffolding | Cursor | Verified file layout, Makefile, sample JSON |
| Module implementation | Cursor | Reviewed logic, ran builds, ran manual tests |
| Debugging | Cursor + manual | Reproduced failures, validated fixes |
| Documentation | Cursor | Edited for accuracy against actual behavior |
| Refactoring & cleanup | Cursor | Confirmed no behavior change after deduplication |
| Code review | Cursor | Used to cross-check requirements coverage |

---

## Where AI assisted

### 1. Scaffolding

- Proposed initial folder structure (`include/`, `src/`, `data/`)
- Generated stub headers, empty source files, `Makefile`, and sample JSON files
- Created the initial `README.md` skeleton

### 2. Architecture and design

- Drafted the simplified control-plane architecture after scope was narrowed (no data plane)
- Suggested module boundaries: `ip_addr`, `route`, `iface`, `rtable`, `lookup`, `json_input`, `events`, `cli`
- Recommended implementation order and shared responsibilities (e.g. `rtable` owns connected-route derivation)
- Produced architecture review notes before implementation began

### 3. Code generation

AI drafted initial implementations for:

- `ip_addr` — IPv4 parsing and prefix math
- `route` — route entry model and builders
- `rtable` — routing table, LPM, connected-route sync
- `iface` — interface list and UP/DOWN state
- `json_input` / `json_util` — JSON loading for interfaces and static routes
- `events` — event load, apply, and replay
- `lookup` — LPM wrapper and lookup explanation output
- `cli` — interactive command loop
- `main` — startup wiring and command-line flags

Temporary test harnesses in `main.c` were also AI-generated during module bring-up and later removed.

### 4. Refactoring suggestions

- Identified duplicated JSON parsing between `json_input.c` and `events.c`; led to extraction of `json_util`
- Flagged unused symbols (`vr_route_init`, public `vr_events_replay_next`, etc.) during cleanup pass
- Suggested consolidating display logic in the CLI module instead of duplicating print code in `main.c`

### 5. Documentation

AI helped draft and iterate on:

- `README.md` — usage, CLI, JSON formats, examples
- `DESIGN.md` — architecture, data flow, LPM algorithm, event replay
- `VALIDATION.md` — build/run steps, expected outputs, validation checklist
- `AI_USAGE.md` — this file

### 6. Code review

- Reviewed proposed architecture against assessment requirements
- Checked requirement coverage (interfaces, connected routes, static routes, events, LPM, CLI)
- Suggested small design corrections (shared UP/DOWN path, `rtable` ownership of connected routes)

---

## Where human verification was performed

The developer manually verified the following before accepting any AI output:

### Build and compile

- Ran `make clean && make` after each significant change
- Confirmed `-Wall -Wextra` builds with no errors
- Fixed compile issues (e.g. missing `#include <stddef.h>`, `#include <errno.h>`)

### Manual testing

- Ran `./build/virtual_routr` with sample data in `data/`
- Verified JSON loading for interfaces, static routes, and events
- Confirmed startup event replay output and final interface/route state
- Tested command-line flags: `-i`, `-r`, `-e`, `-p`, `-d`
- Exercised CLI commands: `show interfaces`, `show routes`, `lookup`, `replay`, `help`, `quit` / `q`
- Validated lookup results for specific destinations (connected, static, default route)
- Confirmed invalid input handling (bad IPv4 addresses, unknown CLI commands)

### Debugging

- Diagnosed and fixed JSON array iteration bug (comma handling between objects)
- Fixed event replay double-load issue in CLI `replay` command
- Corrected `-p` flag behavior and lookup explanation output
- Verified inactive connected routes when interfaces are DOWN

### Code review and acceptance

- Read each module before keeping it; rejected or edited AI suggestions that were over-engineered or out of scope
- Removed temporary test code from `main.c` after module validation
- Removed unused functions and duplicate JSON parsing during cleanup pass
- Confirmed behavior matched assessment requirements, not just AI assumptions

### Documentation accuracy

- Ran documented commands and compared output to `VALIDATION.md` examples
- Updated docs when behavior differed from initial AI drafts (e.g. CLI quit aliases, `-p` semantics)
- Ensured JSON field descriptions match the actual sample files in `data/`

---

## What AI did not do

- AI did **not** run the final acceptance on behalf of the developer; all smoke tests were run locally
- AI did **not** deploy or integrate with external systems; the project is self-contained
- AI was **not** used for requirements the project explicitly excludes (packet forwarding, ARP, ICMP, raw sockets)
- No code was merged without the developer reading and understanding it

---

## Tools used

| Tool | Usage |
|------|-------|
| **Cursor** | Primary IDE; inline code generation, editing, terminal builds, file scaffolding |
| **ChatGPT-style models (via Cursor)** | Architecture discussion, design review, documentation drafting |

---

## Statement

AI tools accelerated scaffolding, implementation drafts, documentation, and review feedback. The final codebase reflects deliberate human choices about scope, structure, and correctness. Every module was built, executed, observed, and corrected manually before being considered complete.
