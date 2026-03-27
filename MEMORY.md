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

## Latest Inspector Slice
- On `2026-03-27`, the inspector-style docks received another bounded modernization pass:
  - `PropertiesDock` now shows a summary card that explains the current canvas or selection context instead of opening as a blank legacy form
  - stitch color/background controls in `PropertiesDock` now read like labeled swatches instead of anonymous icon buttons
  - destructive and media-path actions in `PropertiesDock` now have clearer labels and tooltips
  - `RowsDock` now uses explicit text alignment controls for row and stitch alignment instead of tooltip-only empty radio buttons
  - the dead `Create Rows` checkbox was removed from `RowsDock`; the dock now exposes the two real actions directly as `Create Grid` and `Arrange Selection`
- This slice is centered in:
  - `src/propertiesdock.ui`
  - `src/propertiesdock.cpp`
  - `src/rowsdock.ui`
  - `resources/themes/desktop.qss`

## Latest Settings Slice
- On `2026-03-27`, `SettingsUi` received a compact shell modernization pass:
  - the settings dialog now opens with a summary card that explains the active tab instead of dropping the user directly into an unlabeled old tab layout
  - the summary updates as the user switches between `Application`, `Charts`, `Legends`, and `Tools`
  - outer dialog spacing is less cramped, improving readability on macOS
  - the default-folder picker now uses a readable `Choose...` label and a tooltip instead of an opaque `...` button
- This slice is centered in:
  - `src/settings.ui`
  - `src/settingsui.cpp`
  - `resources/themes/desktop.qss`

## Latest Library Slice
- On `2026-03-27`, `StitchLibraryUi` received the same shell-modernization treatment:
  - the dialog now opens with a summary card for the active stitch set instead of dropping directly into the old table/form split
  - the summary updates as the user switches between the master set and custom sets
  - the old `More >>` affordance was replaced by a clear details toggle for stitch-set metadata
  - the filter row now uses a readable `Clear` affordance instead of an opaque `...` button
  - the action rail now reads more like an explicit control surface instead of a blind button stack
- This slice is centered in:
  - `src/stitchlibrary.ui`
  - `src/stitchlibraryui.cpp`
  - `resources/themes/desktop.qss`

## Latest Row-Edit Slice
- On `2026-03-27`, `RowEditDialog` received a small shell cleanup:
  - the widget now has an explicit summary card instead of opening as an unlabeled mini-form
  - preview and update affordances now explain that they operate on the currently selected row and current chart selection
  - add/remove/move row controls now use readable text labels with tooltips instead of raw symbol-only button text
- This slice is centered in:
  - `src/roweditdialog.ui`
  - `resources/themes/desktop.qss`

## Latest Properties Slice
- On `2026-03-27`, `PropertiesDock` received the first real structure pass instead of reading like a raw legacy form:
  - the dock now shows a persistent summary card describing the current canvas or selection context
  - section titles now adapt to the active selection type instead of leaving anonymous group boxes
  - destructive and file/color actions now use clearer labels and tooltips
  - stitch color controls are now styled as explicit swatch buttons with text beside the preview icon
- This slice is centered in:
  - `src/propertiesdock.ui`
  - `src/propertiesdock.cpp`
  - `resources/themes/desktop.qss`

## Latest Rows Slice
- On `2026-03-27`, `RowsDock` was also pulled forward from the older raw-form layout:
  - the dock now has an explicit summary line and grouped sections for grid size, alignment, spacing, and actions
  - blank radio-button clusters were replaced with labeled tool buttons, making alignment intent readable without relying on tooltips alone
  - dock-level spacing now matches the newer shell and properties panels
- This slice is centered in:
  - `src/rowsdock.ui`
  - `resources/themes/desktop.qss`

## Current Audit Emphasis
- Remaining desktop regression focus is now concentrated on:
  - mouse click consistency across selection, move, stitch, and indicator flows
  - layer-aware grouping and transform behavior under real user sequences
  - more modern, more explicit affordances in `MainWindow` and `PropertiesDock`

## Latest Mouse Consistency Slice
- On `2026-03-27`, pointer interaction handling was tightened again for editor consistency:
  - legacy exact-button comparisons in `Scene` now use bitmask checks, so drag/select behavior no longer depends on the left button being the only pressed button in the event state
  - `Move Edit`, `Color Edit`, `Indicator Edit`, and the shared selection-band path now treat left-button drags consistently under modifier usage and mixed input states
  - `ChartView` edge auto-scroll now also keys off left-button presence rather than exact equality, reducing missed auto-scroll during real drag sequences
- This slice is centered in:
  - `src/scene.cpp`
  - `src/chartview.cpp`

## Latest New-Chart Slice
- On `2026-03-27`, the `New Chart Options` shell received a bounded clarity pass:
  - the flow now shows a persistent summary card instead of forcing the user to mentally assemble chart style, template, preset, and base behavior from scattered controls
  - the summary updates live for custom charts and for `Granny Square` parameters including preset, start type, rounds, and corner arches
  - this keeps the new granny-square template readable without changing the existing file format or document-creation flow
- This slice is centered in:
  - `src/mainwindow.cpp`
  - `src/mainwindow.h`
  - `resources/themes/desktop.qss`

## Latest Resize Slice
- On `2026-03-27`, `ResizeUI` received a bounded modernization and clarity pass:
  - the dock now exposes a more explicit canvas-bounds summary instead of acting like four raw spinboxes with unclear meaning
  - empty-state handling is now explicit when no chart tab is open, instead of leaving stale resize controls active
  - edge labels and control sizing now read more clearly, and the resize hint stays aligned with the current immediate-apply behavior
- This slice is centered in:
  - `src/resize.ui`
  - `src/resizeui.cpp`
  - `src/resizeui.h`
  - `resources/themes/desktop.qss`

## Latest Resize Slice
- On `2026-03-27`, `ResizeUI` received a shell clarity pass:
  - the dock now explains that it edits canvas bounds instead of showing a raw four-field form
  - a live summary reports the current canvas size, so top/bottom/left/right values are not detached from the resulting working area
  - `Clamp to Chart` was renamed to `Fit to Visible Items`, matching what the code actually does and reducing ambiguity for users
- This slice is centered in:
  - `src/resize.ui`
  - `src/resizeui.cpp`
  - `resources/themes/desktop.qss`

## macOS Launch Reliability
- A shell-launched app on this Mac can inherit Homebrew Qt environment contamination from terminal or VSCode shells.
- This causes mixed bundle/Homebrew Qt frameworks before `main()` and can abort in platform plugin initialization.
- The repo now includes `utils/prepare_macos_bundle.sh` and a `bundle_macos` target to deploy the app and install a sanitized launcher script that clears `DYLD_*` and `QT_*` before execing the real Qt binary.

## Latest Packaging Slice
- On `2026-03-27`, macOS bundle sanitization was tightened again after a fresh crash log from a synced user-space app copy:
  - the fatal stack still matched the pre-main mixed-Qt startup failure in `QGuiApplicationPrivate::createPlatformIntegration()`
  - bundle plugin binaries still carried Homebrew-origin install IDs such as `/usr/local/opt/qt@5/plugins/...`, which made shell-launched copies vulnerable to loading mixed Qt components even with the sanitized launcher in place
  - `utils/prepare_macos_bundle.sh` now rewrites Homebrew Qt plugin/framework references back to bundled `@executable_path/../PlugIns/...` and `@executable_path/../Frameworks/...`
  - plugin install IDs inside `Contents/PlugIns` are now normalized to bundled relative paths as part of packaging
  - the bundle packaging script now runs quietly again under `zsh` via `setopt typesetsilent`

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
