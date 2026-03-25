# TODO

## Phase 0. Project Operating System
- [x] Add `AGENTS.md`
- [x] Add `MEMORY.md`
- [x] Add `TODO.md`
- [x] Start repository-local skills for repeatable audit and modernization work
- [x] Keep these files current after every significant slice

## Phase 1. Desktop Feature Audit
- [x] Build an initial matrix of menu actions, toolbar buttons, dock actions, and editing modes
- [ ] Record expected behavior, actual behavior, and failure/risk level for each feature
- [ ] Verify:
  - [ ] grouping / ungrouping
  - [ ] locking / fixation semantics if present
  - [ ] mirror / copy / rotate
  - [ ] align / distribute
  - [ ] rows / arrange grid
  - [ ] guidelines and backdrops
  - [ ] layer actions
  - [ ] properties editing
  - [ ] import / export / print / save flows
- [x] Add a reproducible desktop regression checklist document

## Phase 2. Desktop Stability
- [ ] Remove high-risk dangling-pointer and lifetime bugs in scene/dock interactions
- [ ] Add safer selection and tab-change handling
- [ ] Add internal logging hooks for user action failures where behavior is silent today
- [ ] Restore or replace the broken legacy test entry point under `tests/`
- [ ] Finish the remaining P0 action fixes after the first behavior pass:
  - [x] `RowsDock` vertical alignment mapping
  - [x] `Color Edit` checked-state sync
  - [x] selected-item `Arrange` implementation
  - [x] reduce multi-select rotate / scale undo corruption risk
  - [x] `Ctrl` additive selection reliability
  - [x] indicator paste undo path
  - [x] mixed non-cell properties behavior

## Phase 3. Desktop UI Foundation
- [ ] Introduce a shared Qt theme layer:
  - [ ] typography scale
  - [ ] spacing scale
  - [ ] surface colors
  - [ ] glass overlay treatment
  - [ ] button states
  - [ ] dock chrome
  - [ ] form controls
- [ ] Apply the theme first to:
  - [ ] main window shell
  - [ ] new chart dialog
  - [ ] properties dock
  - [ ] align dock
  - [ ] mirror dock
  - [ ] settings
- [ ] Replace the most legacy-feeling icons and control layouts

## Phase 4. Desktop Workflow Modernization
- [ ] Rework the new chart flow around templates and presets
- [ ] Make backdrop/guideline configuration clearer and less fragile
- [ ] Improve discoverability of edit modes and selection modes
- [ ] Clarify destructive actions and selection-dependent controls

## Phase 5. Shared Core Extraction
- [ ] Identify pure model/domain logic that can move out of widget classes
- [ ] Define reusable chart document, selection transform, and template services
- [ ] Create a boundary suitable for desktop, web, and Android reuse

## Phase 6. Online / VDS Product
- [ ] Create `server/` architecture plan
- [ ] Define auth model:
  - [ ] login/password
  - [ ] session management
  - [ ] account state
- [ ] Define subscription model:
  - [ ] 1 month
  - [ ] 3 months
  - [ ] 1 year
  - [ ] annual price target `5000 RUB`
- [ ] Define persistence and API for charts and user assets
- [ ] Define deployment baseline for VDS

## Phase 7. Android
- [ ] Decide delivery strategy:
  - [ ] Qt for Android
  - [ ] packaged web client
  - [ ] hybrid shell over online app
- [ ] Produce a buildable APK path
- [ ] Align mobile UX with the same document model and account system

## Phase 8. Release
- [ ] Desktop release candidate with audited major actions
- [ ] Online alpha on VDS with auth and subscriptions
- [ ] Android alpha build
- [ ] Repo cleanup, release notes, and branch strategy review
