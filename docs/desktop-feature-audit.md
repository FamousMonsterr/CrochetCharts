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
- box / lasso / line selection with modifier keys
- color edit drag behavior
- move-mode click/drag consistency on real documents
- layer change side-effects on selection and transform actions

### Probably Broken or Not Implemented
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
- Fixed in the current interaction slice:
  - `Move Edit` is now exposed as a first-class mode with explicit cursor feedback during idle and drag states
  - `Snap to grid` now has an explicit toggle, on/off feedback, and disabled-state explanation when no grid guide is active
  - switching or editing layers now refreshes grouping availability instead of leaving stale action state behind
  - cross-layer grouping now surfaces a readable reason instead of degrading into an unclear no-op
  - group / ungroup undo-redo paths now keep layer-gated selectability rules intact
- Fixed in the current shell slice:
  - the desktop shell now shows persistent chart / mode / select / grid / layer / selection context instead of relying only on transient status-bar messages
  - selection-mode actions now re-synchronize on tab switch
  - icon-only selection, grid, and layer controls now expose clearer tooltips
  - the layers dock no longer hides its own visibility/name headers
  - align/distribute and copy/mirror/rotate docks now expose their selection prerequisites directly instead of reading like always-on controls
- Still requiring explicit manual regression:
  - move-mode drag and click behavior on dense charts
  - `Ctrl` additive selection
  - indicator paste undo
  - mixed-property handling for non-cell multi-selection
  - group / ungroup enablement and warning coverage across layer changes
  - directional copy / mirror with mixed selections containing indicators or chart center

## Manual QA Checklist
- Switch between `Move Edit`, `Stitch Edit`, and `Indicator Edit`; confirm cursor and click behavior make the active mode obvious.
- Turn `Snap to grid` on and off in square, round, triangle, and no-grid charts; confirm the action availability and actual placement match the toggle state.
- Select a layer, then change layer visibility and confirm `Group` / `Ungroup` action state updates immediately.
- Try grouping items from one layer, then from multiple layers after changing active layer state; confirm the UI reports the constraint clearly.
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
