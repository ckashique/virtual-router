# AI Prompt Log — virtual_routr

This file records the **user prompts** submitted to Cursor during development of this project, in chronological order. Prompts are reproduced as entered (minor formatting normalized for readability).

For how AI output was reviewed and corrected, see `AI_USAGE.md`.

---

## Prompts

### 1. Initial architecture (full router — later revised)

```
I'm building a small IPv4 virtual router in C for Linux.

First, review the assessment requirements and propose a simple project architecture.

Include:
- Folder structure
- Source files
- Header files
- Brief responsibility of each module
- Suggested implementation order

Keep the design simple and focused only on the assessment requirements.

Don't generate any code yet.

(Use curremt directory for code : /Users/ashck/Documents/interview-new/virtual_routr)
```

---

### 2. Simplify architecture (control plane only)

```
This is more complex than what I need.

The assessment is not asking for packet forwarding or Ethernet processing.

Let's simplify the design.

The router only needs to:
- Load interfaces from JSON
- Derive connected routes
- Load static routes
- Maintain a routing table
- Handle interface UP/DOWN events
- Replay events from JSON
- Perform longest prefix match
- Explain route lookup decisions
- Provide a CLI

Please redesign the architecture around these requirements only.

Don't include:
- Ethernet
- ARP
- ICMP
- Packet forwarding
- Raw sockets
- TUN/TAP
- Checksums

Don't generate code yet.
```

---

### 3. Architecture review before implementation

```
Looks good.

Before we start implementation, review this architecture one more time against the assessment requirements.

Specifically verify:
- Every assessment requirement is covered.
- No unnecessary modules are included.
- The module responsibilities are clear.
- The implementation order is reasonable.

If you find anything missing, suggest only small changes.

Don't generate any code.
```

---

### 4. Create project scaffold

```
Create the project structure based on this architecture.
Add the folders, empty source files, header files, sample JSON files, Makefile and README.
Don't implement any functionality yet.
```

---

### 5. Implement `ip_addr` module

```
Let's start with IP address handling.
Implement ip_addr.c and ip_addr.h.
It should support:
- Parse IPv4 address strings.
- Convert prefix length to subnet mask.
- Calculate network address.
- Check whether an IP belongs to a network.

Add only this module.
Don't implement anything else.
```

---

### 6. Add temporary `ip_addr` tests

```
Add some basic tests for ip_addr in main().

Test:
- valid IPv4 parsing
- invalid IPv4 parsing
- network address calculation
- IP inside and outside a subnet
- prefix /0 and /32
```

---

### 7. Remove temporary tests

```
The tests look good.
Remove the temporary test code and keep only the module implementation.
```

---

### 8. Implement `route` module

```
Implement the route data structures.
Support connected and static routes.
Don't implement the routing table yet.
```

---

### 9. Add temporary `route` tests

```
Add a few tests for the route module.

Please cover:
- connected route
- static route
- route match
- route doesn't match
- invalid prefix
```

---

### 10. Remove temporary tests

```
The tests look good.
Remove the temporary test code.
```

---

### 11. Implement `rtable` module

```
Implement the routing table in rtable.c and rtable.h.

Support:
- initialize the table
- add connected route
- add static route
- list all routes
- longest prefix match (active routes only)

Keep it in-memory only. No JSON or CLI yet.

Add a few basic tests in main()
```

---

### 12. Implement `iface` module

```
Implement the interface module.

Each interface should have:
- name
- IPv4 address
- prefix length
- UP/DOWN state

Add functions to create an interface, change its state, and retrieve it by name.

Add a few basic tests in main().
```

---

### 13. Connect `iface` and `rtable`

```
Connect the interface and routing table modules.
Add a function that builds connected routes from all UP interfaces.
If an interface goes DOWN, its connected route should become inactive.
If it comes UP again, the route should become active again.
Add a few basic tests in main()
```

---

### 14. JSON loading for interfaces and static routes

```
Let's load interfaces and static routes from JSON.
Use the sample files in data/.
Don't add test code in main(). Just make sure the data loads correctly.
```

---

### 15. Implement `lookup` module

```
Implement the lookup module. Add vr_lookup_route() in lookup.h/.c that takes a destination IPv4 address and returns the best matching route using vr_rtable_lpm(). Also add a helper to print the selected route in a readable format. Do not modify other modules.
```

---

### 16. Implement `events` module

```
Implement the events module only. Load events from events.json and provide an API to replay them one by one. For each iface_up or iface_down event, update the interface state using vr_iface_set_state() and refresh the routing table using vr_rtable_on_iface_change(). Print each event as it is processed. Do not modify the CLI yet. Update main() only enough to replay all events after loading the configuration.
```

---

### 17. Add `-d` command-line option

```
Add a -d <IPv4> command-line option. Parse the destination IP using vr_ipv4_parse(), perform longest-prefix-match using vr_lookup_route(), and print the selected route. If no route matches, print an appropriate message. Keep the existing functionality unchanged.
```

---

### 18. Add `-p` command-line option

```
Add a -p option that prints the current interfaces and routing table after loading configuration and replaying events. If -d is also given, print the lookup result after the tables. Keep the existing behavior unchanged.
```

---

### 19. Improve lookup explanation output

```
Update the lookup module to explain how the route was selected. For each active route, show whether it matched the destination and its prefix length. Finally print why the selected route won (longest prefix match), or say that no route matched. Don't change the lookup logic. Just improve the output.
```

---

### 20. Implement CLI

```
Implement the CLI in cli.c. Support these commands: show interfaces, show routes, lookup <ip>, replay, help, and quit. Reuse the existing modules instead of duplicating logic.
```

---

### 21. Code cleanup pass

```
Review the project for any duplicate code, unused functions, unused headers, debug prints, or temporary code left during development. Remove anything unnecessary without changing the program behavior.
Also remove if any test cases are added to just testing of code.
```

---

### 22. Add `q` as quit alias

```
vr> q
Unknown command: q

Please add q also to quit.
```

---

### 23. Update README

```
Update the README with architecture, module responsibilities, build instructions, CLI commands, JSON formats, and a few example runs. Do not change any C source files.
```

---

### 24. Create DESIGN.md

```
Create DESIGN.md describing the architecture of the virtual router. Include module responsibilities, data flow, routing table design, longest-prefix-match algorithm, event replay flow, and why each module exists. Do not modify any C source files.
```

---

### 25. Create VALIDATION.md

```
Create VALIDATION.md describing how to build, run, and manually validate the project. Include the commands used, expected outputs, CLI commands to verify, lookup examples, event replay verification, and a checklist confirming the project works as expected. Do not modify any C source files.
```

---

### 26. Create AI_USAGE.md (initial version)

```
Create AI_USAGE.md describing how AI tools (Cursor/ChatGPT) were used during development. Mention that AI was used for scaffolding, code review, documentation, and refactoring suggestions, while all code was manually reviewed, built, tested, debugged, and understood before acceptance. Include a brief list of where AI assisted and where human verification was performed. Do not modify any C source files.
```

---

## Summary

| Metric | Count |
|--------|------:|
| Total prompts | 26 |
| Implementation prompts | 14 |
| Test / cleanup prompts | 6 |
| Documentation prompts | 5 |
| Design / review prompts | 3 |
