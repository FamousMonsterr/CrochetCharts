# AGENTS

## Purpose
This repository now uses explicit workstreams so the desktop product can be stabilized first and then expanded into web and Android without losing control of scope.

## Working Rules
- Preserve existing user changes unless the task explicitly requires replacing them.
- Prefer small, testable slices over broad rewrites.
- Every substantial change should update [MEMORY.md](MEMORY.md) and, when scope changes, [TODO.md](TODO.md).
- Desktop correctness comes before visual polish.
- Web/VDS and Android work must reuse a shared domain model where practical instead of forking product behavior.
- Current execution order for modernization is:
  - stabilize editor behavior
  - finish the desktop UI system pass
  - extract shared document/core boundaries
  - only then expand into online and Android delivery

## Sub-Agent Workflow
- Use sub-agents for bounded micro-tasks:
  - behavior audits
  - code-path tracing
  - architecture spikes
  - disjoint implementation slices
- Prefer explorer-style sub-agents before editing shared desktop behavior when a path is risky or unclear.
- Close completed sub-agent threads and open fresh ones for new micro-slices instead of leaving stale sessions hanging.
- Record sub-agent conclusions back into `MEMORY.md`, `TODO.md`, and the relevant audit doc before moving to the next slice.
- Main Integrator keeps final responsibility for merge quality, verification, and push decisions.

## Agent Roles

### 1. Main Integrator
- Owns roadmap, merge decisions, repo hygiene, release notes, and final verification.
- Decides the next implementation slice and keeps `MEMORY.md` current.

### 2. Desktop Behavior Audit
- Verifies what every action, toolbar button, dock control, and mode switch actually does.
- Focus areas:
  - selection modes
  - grouping / ungrouping
  - mirroring / copy / rotate
  - guidelines and chart backdrops
  - properties editing
  - rows / align / resize tools
  - layers
- Produces reproducible manual test steps and marks behaviors as correct, unclear, broken, or risky.

### 3. Desktop UI Modernization
- Owns the visual system for Qt desktop.
- Must keep the UI flat, precise, readable, and restrained.
- Target style:
  - flat surfaces
  - subtle glass overlays where justified
  - no gradients
  - no generic AI-generated ornament
  - tighter spacing, clearer hierarchy, better icon consistency

### 4. Desktop Core Refactor
- Extracts reusable business logic from widgets where possible.
- Reduces unsafe object ownership, signal/slot fragility, and scene/view coupling.
- Adds test seams and internal instrumentation for regressions.

### 5. Web/VDS Platform
- Defines the server-side product shape:
  - authentication
  - subscription lifecycle
  - chart persistence
  - collaboration and online editing boundaries
- Must avoid binding the business model directly to desktop widgets.

### 6. Android Product
- Defines the mobile product scope and packaging path.
- Must not start as a full rewrite until shared core and API boundaries are clear.
- Acceptable staged outcomes:
  - packaged web client
  - Qt for Android client
  - native shell over shared API

### 7. Billing and Access
- Owns pricing logic, plan durations, entitlements, and renewal rules.
- Current business requirement:
  - 1 month
  - 3 months
  - 1 year
  - annual plan is the best value and 40% cheaper than paying monthly for 12 months
  - annual target price is 5000 RUB

### 8. Release and QA
- Builds desktop bundles.
- Runs regression passes on high-risk actions.
- Verifies installed app behavior on macOS.
- Verifies shell-launched macOS bundles do not inherit broken Qt runtime state from terminal or editor environments.

## Current Ownership Map
- `src/mainwindow.*`, `src/*.ui`, `src/*dock*`, `src/settings*`, `src/chartview*`: Desktop UI Modernization + Desktop Behavior Audit
- `src/scene.*`, `src/crochettab.*`, `src/crochetchartcommands.*`, `src/file*.cpp`: Desktop Core Refactor + Desktop Behavior Audit
- `docs/*`, `README.md`, `translations/*`: Main Integrator
- future `server/`, `web/`, `android/`, `shared/`: respective platform owners

## Expected Deliverables
- Behavior matrix for every user-facing action.
- Modernized Qt theme foundation and updated key workflows.
- Shared-core extraction plan.
- Auth/subscription architecture for online mode.
- Android packaging strategy backed by actual shared logic, not a disconnected fork.
