# virtual_routr — Validation Guide

This document describes how to build, run, and manually validate **virtual_routr** using the sample data in `data/`. All commands assume you are in the project root directory.

---

## Prerequisites

- `gcc` or `clang` with C11 support
- `make`
- Sample JSON files in `data/` (included in the repository)

---

## 1. Build

### Commands

```bash
make clean
make
```

### Expected result

- Build completes with no errors or warnings
- Binary is created at `build/virtual_routr`

```bash
ls -l build/virtual_routr
```

You should see an executable file.

### Clean rebuild (optional)

```bash
make clean && make
```

---

## 2. Basic run

### Command

```bash
./build/virtual_routr -p
```

The `-p` flag skips the pre-replay table dump and prints state once after events are replayed.

### Expected output

```
Replaying 3 event(s)...
event seq=1 type=iface_down interface=eth1
event seq=2 type=iface_up interface=eth2
event seq=3 type=iface_up interface=eth1
Interfaces (3):
  eth0  192.168.1.1/24  up
  eth1  10.0.0.1/24  up
  eth2  172.16.0.1/16  up
Routes (5):
  connected  192.168.1.0/24  iface=eth0  active
  connected  10.0.0.0/24  iface=eth1  active
  connected  172.16.0.0/16  iface=eth2  active
  static  0.0.0.0/0  iface=eth0  active  via 192.168.1.254
  static  203.0.113.0/24  iface=eth1  active  via 10.0.0.254
Type 'help' for commands.
vr>
```

### What to verify

- [ ] Three startup events are replayed in sequence order
- [ ] Three interfaces are listed; all show `up` after replay
- [ ] Five routes are listed (three connected, two static)
- [ ] All routes show `active` after replay
- [ ] Program waits at the `vr>` prompt

Exit with `quit`, `exit`, or `q`.

---

## 3. Pre-replay vs post-replay state

Without `-p`, tables are printed **before** and **after** event replay.

### Command

```bash
./build/virtual_routr
```

Type `q` at the prompt to exit.

### Expected differences

**Before replay** (from initial JSON):

| Item | Expected value |
|------|----------------|
| `eth2` state | `down` |
| Connected route `172.16.0.0/16` (eth2) | `inactive` |
| `eth0`, `eth1` | `up` with active connected routes |

**After replay** (from `data/events.json`):

| Event | Effect |
|-------|--------|
| seq 1: `iface_down eth1` | eth1 briefly down during replay (final state restored by seq 3) |
| seq 2: `iface_up eth2` | eth2 becomes up; connected route becomes active |
| seq 3: `iface_up eth1` | eth1 up again |

Final state matches section 2 above.

### What to verify

- [ ] Pre-replay dump shows `eth2` as `down` and its connected route as `inactive`
- [ ] Post-replay dump shows `eth2` as `up` and its connected route as `active`
- [ ] Without `-p`, two table dumps appear; with `-p`, only the post-replay dump appears

---

## 4. Command-line lookup (`-d`)

### Command

```bash
./build/virtual_routr -p -d 203.0.113.5
```

Type `q` to exit the CLI afterward.

### Expected lookup section

```
Route lookup for 203.0.113.5:
  connected  192.168.1.0/24  prefix_len=24  matched=no
  connected  10.0.0.0/24  prefix_len=24  matched=no
  connected  172.16.0.0/16  prefix_len=16  matched=no
  static  0.0.0.0/0  prefix_len=0  matched=yes
  static  203.0.113.0/24  prefix_len=24  matched=yes
Selected route:
  static  203.0.113.0/24  iface=eth1  active  via 10.0.0.254
Reason: longest prefix match (/24 is the longest matching prefix among active routes).
```

### What to verify

- [ ] Both `/0` and `/24` static routes match
- [ ] `/24` static route is selected over the default route
- [ ] Explanation lists all active routes with `matched=yes/no`

### Invalid address

```bash
./build/virtual_routr -d not-an-ip
echo $?
```

Expected:

```
Invalid destination IPv4: not-an-ip
```

Exit code: `1`

---

## 5. CLI command verification

Run an interactive session by piping commands:

```bash
./build/virtual_routr -p <<'EOF'
help
show interfaces
show routes
lookup 192.168.1.50
lookup 100.64.0.1
q
EOF
```

### `help`

Expected: list of commands including `show interfaces`, `show routes`, `lookup <ip>`, `replay`, `help`, `quit`.

### `show interfaces`

Expected:

```
Interfaces (3):
  eth0  192.168.1.1/24  up
  eth1  10.0.0.1/24  up
  eth2  172.16.0.1/16  up
```

### `show routes`

Expected: five routes matching the post-replay table in section 2.

### Unknown command

```
vr> foo
Unknown command: foo
```

Help text is printed to stderr.

---

## 6. Lookup examples

Use the sample routing table after startup replay.

### Example A — connected route wins (longer than default)

**Command:** `lookup 192.168.1.50`

**Expected selected route:**

```
connected  192.168.1.0/24  iface=eth0  active
```

**Why:** `/24` connected route matches and beats the `/0` default route.

---

### Example B — specific static route

**Command:** `lookup 203.0.113.5`

**Expected selected route:**

```
static  203.0.113.0/24  iface=eth1  active  via 10.0.0.254
```

**Why:** `/24` static route is a longer match than `/0`.

---

### Example C — default route

**Command:** `lookup 100.64.0.1`

**Expected selected route:**

```
static  0.0.0.0/0  iface=eth0  active  via 192.168.1.254
```

**Why:** No more-specific route matches; default route applies.

---

### Example D — invalid CLI lookup

**Command:** `lookup not-an-ip`

**Expected:**

```
Invalid destination IPv4: not-an-ip
```

Program stays in the CLI.

---

## 7. Event replay verification

### Startup replay

Already covered in section 2. Events from `data/events.json`:

| seq | type | interface |
|-----|------|-----------|
| 1 | iface_down | eth1 |
| 2 | iface_up | eth2 |
| 3 | iface_up | eth1 |

Each event is printed as:

```
event seq=N type=iface_up|iface_down interface=ethN
```

### CLI replay

```bash
./build/virtual_routr -p <<'EOF'
show routes
replay
show routes
q
EOF
```

**Expected:**

1. Initial `show routes` prints five routes (all active after startup replay)
2. `replay` prints `Replaying 3 event(s)...` and the three event lines again
3. Second `show routes` shows the same five routes (state is idempotent when replaying the same events from the same final state)

### Verify connected route reacts to interface state

To observe inactive connected routes via replay, use the pre-replay state (section 3) or temporarily edit `data/events.json` to leave an interface down. With the stock sample data:

- Before startup replay completes, `eth2` connected route is `inactive`
- After event seq 2 (`iface_up eth2`), it becomes `active`

---

## 8. Custom config paths

```bash
./build/virtual_routr \
  -i data/interfaces.json \
  -r data/static_routes.json \
  -e data/events.json \
  -p
```

### What to verify

- [ ] Same behavior as default paths
- [ ] Missing file produces an error and non-zero exit, e.g.:

```bash
./build/virtual_routr -i missing.json
echo $?
```

Expected: error message about failed load, exit code `1`.

---

## 9. Validation checklist

Use this checklist to confirm the project works end-to-end.

### Build

- [ ] `make clean && make` succeeds with no warnings
- [ ] `build/virtual_routr` exists and is executable

### Configuration loading

- [ ] Interfaces load from `data/interfaces.json` (3 interfaces)
- [ ] Static routes load from `data/static_routes.json` (2 routes)
- [ ] Connected routes are derived (3 connected + 2 static = 5 total)

### Event replay

- [ ] Startup replays 3 events in seq order
- [ ] Each event is printed during replay
- [ ] Interface state changes are reflected (eth2 up after replay)
- [ ] Connected route active flags follow interface UP/DOWN

### Routing table

- [ ] `show routes` lists connected and static routes
- [ ] Connected routes use normalized network prefixes (e.g. `192.168.1.0/24`)
- [ ] Static routes show next-hop addresses

### Longest-prefix match

- [ ] `lookup 192.168.1.50` selects connected `/24` on eth0
- [ ] `lookup 203.0.113.5` selects static `/24` on eth1
- [ ] `lookup 100.64.0.1` selects default `/0` on eth0
- [ ] Lookup explanation shows match results for all active routes
- [ ] `-d` flag produces the same lookup result from the command line

### CLI

- [ ] `help` lists all commands
- [ ] `show interfaces` and `show routes` work
- [ ] `lookup <ip>` works
- [ ] `replay` reloads and replays events
- [ ] `q`, `quit`, and `exit` all terminate the program
- [ ] Unknown commands print an error without crashing

### Error handling

- [ ] Invalid `-d` address exits with code 1
- [ ] Invalid CLI lookup prints an error and continues
- [ ] Missing JSON file exits with code 1

---

## 10. Quick smoke test (copy-paste)

Run this single script to exercise the main paths:

```bash
make clean && make && \
./build/virtual_routr -p -d 203.0.113.5 <<'EOF'
lookup 192.168.1.50
lookup 100.64.0.1
replay
q
EOF
```

If all sections above pass, the virtual router control plane is functioning as expected.
