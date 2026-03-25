# Desktop Feature Audit

Date: `2026-03-24`

## Scope
- Main window actions and menus
- Edit modes
- Selection modes
- Docks and tool panels
- Guideline and backdrop controls
- Transform operations

## Status Summary

### Implemented Correctly
- dock visibility toggles
- view toggles and fullscreen wrappers
- chart center visibility and movement
- guideline type switching
- scene-level guideline property updates

### Implemented With Caveats
- file flows: wiring is present, deeper save/print/export QA still required
- stitch edit
- indicator edit
- group / ungroup
- align / distribute
- mirror / directional copy / rotate
- mixed chart properties for regular cell selections

### Suspicious
- copy / cut / paste around indicator undo
- box / lasso / line selection with modifier keys
- selectable item hit testing
- color edit drag behavior
- wheel zoom modifier handling
- mixed properties for non-cell multi-selection
- menu enablement for group / ungroup

### Probably Broken or Not Implemented
- `RowsDock -> Arrange` for selected items
- `RowsDock` vertical alignment mapping
- multi-select rotate / scale undo flow
- `Color Edit` checked-state sync
- align to path / distribute to path

## P0 Fix Targets
- Fixed in the first two modernization slices:
  - `RowsDock` vertical alignment mapping in `src/rowsdock.cpp`
  - `Color Edit` checked-state sync in `src/mainwindow.cpp`
  - wheel zoom modifier check in `src/chartview.cpp`
  - `Arrange` implementation for selected items in `src/scene.cpp`
  - reduced risk in multi-select rotate / scale undo flow in `src/scene.cpp`
- Fixed in the current slice:
  - `Ctrl` additive selection now uses modifier bit checks consistently in `src/scene.cpp`
  - indicator paste is now added through the undo stack in `src/scene.cpp`
  - multi-selection of `Indicator`, `ChartImage`, and `ItemGroup` now falls back to safe mixed generic properties in `src/propertiesdock.cpp`
- Fixed in the latest slice:
  - shell-launched macOS bundles are now hardened against mixed Homebrew/bundle Qt runtime loading via `bundle_macos`
  - silent no-op paths now emit diagnostics for empty or invalid selection use in `src/scene.cpp`
  - main-window action entry points now surface missing-tab failures through the status bar instead of silently returning
  - `Group` / `Ungroup` menu enablement now reflects selection state in `src/mainwindow.cpp`
  - `Group` / `Ungroup` action state now updates live on selection change and tab change
  - invalid chart-image path edits now surface an error instead of silently failing in `src/ChartImage.cpp`
- Still requiring explicit manual regression:
  - `Ctrl` additive selection
  - indicator paste undo
  - mixed-property handling for non-cell multi-selection
  - group / ungroup enablement and warning coverage
  - directional copy / mirror with mixed selections containing indicators or chart center

## Manual QA Checklist
- Select several stitches, rotate them, then run `Undo` and `Redo` repeatedly.
- Select several stitches, scale them, then run `Undo` and `Redo` repeatedly.
- Open `RowsDock`, try `Arrange`, confirm selected items really move into a grid.
- In selection modes, add to selection with `Ctrl` and verify previous selection is preserved.
- Paste a selection containing an indicator, then run `Undo`.
- In color edit mode, drag across multiple cells and confirm color paint behavior matches intent.
- Test wheel zoom with `Ctrl`, `Alt`, and `Shift` separately.

## Files Under Highest Scrutiny
- `src/mainwindow.cpp`
- `src/scene.cpp`
- `src/chartview.cpp`
- `src/rowsdock.cpp`
- `src/propertiesdock.cpp`
- `src/crochetchartcommands.cpp`
