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
  - manual regression of the recently fixed selection / paste / properties / group flows
  - remaining dangling-pointer and lifetime issues in scene/dock interactions
  - deeper `PropertiesDock` and `MainWindow` UI modernization
  - expanding the restored test entry point toward selection-transform and file-flow coverage

## Latest Interaction Slice
- On `2026-03-27`, the editor interaction layer was tightened further:
  - `Move Edit` now has explicit cursor feedback instead of reading like the generic default arrow
  - `Snap to grid` now reports clear on/off state in the status bar and explains when it is unavailable because no guide is active
  - layer selection and layer visibility edits now refresh selection-dependent action state immediately
  - `Group` / `Ungroup` entry points now surface readable reasons when the action is unavailable, including cross-layer grouping
  - grouping / ungrouping now respects active-layer selectability rules instead of making foreign-layer items selectable on undo/redo paths
- This slice is centered in:
  - `src/scene.cpp`
  - `src/scene.h`
  - `src/mainwindow.cpp`

## Latest Shell Slice
- On `2026-03-27`, the main desktop shell received a persistent editor context bar:
  - current chart, edit mode, selection mode, grid state, active layer, and selection count are now always visible above the work area
  - selection-mode state now re-synchronizes on tab switch instead of potentially lying after switching charts
  - icon-only selection, grid, and layer controls now have clearer hover help
  - the layers dock now exposes its header and reads more like an explicit control surface instead of a blind icon strip
- This slice is centered in:
  - `src/mainwindow.ui`
  - `src/mainwindow.cpp`
  - `resources/themes/desktop.qss`

## Latest Dock Slice
- On `2026-03-27`, `AlignDock` and `MirrorDock` were made more honest about selection requirements:
  - both docks now show an explicit hint instead of presenting all operations as always available
  - align/distribute actions disable until at least two items are selected
  - copy/mirror/rotate actions disable until there is an actionable selection
  - the dock enable-state is now driven from the same selection refresh pass that updates `Group` / `Ungroup`
- This slice is centered in:
  - `src/aligndock.ui`
  - `src/aligndock.cpp`
  - `src/mirrordock.ui`
  - `src/mirrordock.cpp`
  - `src/mainwindow.cpp`

## Current Audit Emphasis
- Remaining desktop regression focus is now concentrated on:
  - mouse click consistency across selection, move, stitch, and indicator flows
  - layer-aware grouping and transform behavior under real user sequences
  - more modern, more explicit affordances in `MainWindow` and `PropertiesDock`

## macOS Launch Reliability
- A shell-launched app on this Mac can inherit Homebrew Qt environment contamination from terminal or VSCode shells.
- This causes mixed bundle/Homebrew Qt frameworks before `main()` and can abort in platform plugin initialization.
- The repo now includes `utils/prepare_macos_bundle.sh` and a `bundle_macos` target to deploy the app and install a sanitized launcher script that clears `DYLD_*` and `QT_*` before execing the real Qt binary.

## Latest Reliability Slice
- On `2026-03-25`, launch reliability and silent-action diagnostics were extended:
  - macOS bundles can now be wrapped with `bundle_macos` so direct shell launch does not mix bundled Qt with Homebrew Qt
  - editor actions that used to no-op silently now log explicit warnings for empty or invalid selection states in `src/scene.cpp`
  - main-window actions that require an open chart tab now report unavailability through the status bar instead of failing silently
  - `Group` / `Ungroup` menu enablement is now selection-aware in `src/mainwindow.cpp`
  - `Group` / `Ungroup` action state now also refreshes live on tab switches and scene selection changes instead of only when the menu opens
  - changing an existing chart image to an invalid file path now surfaces an error instead of failing silently in `src/ChartImage.cpp`
  - cancelled image pickers no longer create bogus operations in `src/mainwindow.cpp` and `src/propertiesdock.cpp`
  - the legacy `tests/` entry point was replaced with a working Qt5/CTest target, and `ctest --test-dir build --output-on-failure` passes
