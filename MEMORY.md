# MEMORY

## Snapshot
- Project: `Crochet Charts`
- Repository: `FamousMonsterr/CrochetCharts`
- Local branch: `master`
- Upstream branch model still uses `master`; any move to `main` should be handled explicitly and not assumed.
- Date baseline: `2026-03-24`

## Product State
- Legacy desktop application on `Qt 5` / `CMake`.
- Modern macOS build restored and working.
- Russian localization added.
- In-app language switching exists for English and Russian.
- Startup splash was made readable and appears earlier during app boot.
- Granny square template support exists in the new chart flow.
- Low graphics load mode exists for older Macs.

## Installed Desktop Paths
- `/Applications/Crochet Charts.app`
- `/Users/timofey/Applications/Crochet Charts.app`

## Important Recent Functional Changes
- Safer `PropertiesDock` scene ownership to avoid crash during guideline/backdrop selection.
- Runtime warnings around startup font and noisy updater behavior were reduced.
- Granny square documentation and presets were added.

## Known Technical Debt
- The codebase is still heavily widget-coupled.
- `MainWindow`, `Scene`, and dock widgets hold too much behavior directly.
- There is no modern automated regression suite for user actions.
- `tests/CMakeLists.txt` is still `Qt4`-era and is not a working modern test entry point.
- Styling is mostly inherited from old Qt defaults and piecemeal widget settings.
- Desktop features have not yet been exhaustively audited button-by-button.

## Current Modernization Goals

### Desktop
- Verify every action and button against intended behavior.
- Reduce unsafe scene/view interactions.
- Introduce a coherent modern Qt visual system:
  - flat
  - restrained
  - Apple-liquid-glass-inspired overlays where useful
  - no gradients
  - no visual clutter

### Web / VDS
- Online version with login/password authentication.
- Subscription-based access.
- Server-hosted persistence and account model.

### Android
- Deliver an installable Android APK.
- Final packaging path still to be selected after shared core and web strategy are clarified.

## Subscription Requirement
- Annual plan target price: `5000 RUB`
- Annual plan must be `40%` cheaper than paying monthly for 12 months.
- Derived monthly baseline before rounding: about `694.44 RUB`
- Derived 3-month baseline before rounding: about `2083.33 RUB`
- Rounding policy still needs a product decision before implementation.

## Constraints
- Code license: `GPLv3`
- Artwork / graphical assets: `CC BY-SA 4.0`
- Any redistribution must preserve both licensing obligations.

## Key Technical Entry Points
- `src/main.cpp`: application startup, translations, splash, global init
- `src/application.*`: `QApplication` wrapper and file-open flow
- `src/mainwindow.*`: major UI orchestration
- `src/scene.*`: editing behavior and chart manipulation
- `src/crochettab.*`: tab-level document controller
- `src/crochetchartcommands.*`: undoable editing commands
- `src/*dock*.*`, `src/resizeui.*`: dock and tool panels
- `src/file_v1.*`, `src/file_v2.*`, `src/filefactory.*`: file I/O

## Immediate Next Steps
- Build and maintain a feature behavior matrix for user-facing actions.
- Add a shared desktop theme foundation instead of ad hoc widget styling.
- Start untangling behavior from UI where regressions are most likely.
- Decide the first realistic online architecture slice instead of attempting a full rewrite blindly.

## Latest Audit Facts
- Desktop action audit is now recorded in `docs/desktop-feature-audit.md`.
- Confirmed P0 behavior bugs:
  - `RowsDock` vertical alignment mapping
  - `Color Edit` checked-state sync
  - selected-item `Arrange` path is still effectively missing
  - multi-select rotate / scale undo path is high risk
- First modernization slice started with an application theme layer in `src/theme.*` and `resources/themes/desktop.qss`.

## Latest Implementation Slice
- On `2026-03-25`, the next P0 behavior pass was implemented:
  - `RowsDock` is no longer force-disabled in the desktop shell
  - `Scene::arrangeGrid(..., useSelection=true)` now performs real selection layout into a grid with alignment and spacing
  - multi-item rotate / scale no longer performs an extra eager `ungroup()` before the undo command, reducing transform-history corruption risk
  - row-edit mouse handling now checks the left button correctly instead of relying on a precedence bug
- These fixes are implemented in:
  - `src/mainwindow.cpp`
  - `src/scene.cpp`

## Current Focus
- After the latest editor behavior pass, the next high-value targets are:
  - manual regression of the recently fixed selection / paste / properties paths
  - remaining P1 behavior fixes from `docs/desktop-feature-audit.md`
  - deeper `PropertiesDock` and `MainWindow` UI modernization
  - restoring a modern automated regression entry point under `tests/`
