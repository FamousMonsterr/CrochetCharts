# Desktop UI Modernization

## Purpose
Move the Qt desktop UI from legacy utility-tool styling to a coherent modern interface without sacrificing density or clarity.

## Design Direction
- Flat first
- Subtle glass overlays only where they improve hierarchy
- No gradients
- No decorative noise
- Strong icon consistency
- Clear spacing rhythm
- Better legibility than stock Qt widgets

## Target Areas
- main application shell
- new chart flow
- dock chrome
- inspector-style panels
- primary editing toolbars
- settings window

## Workflow
1. Introduce shared theme tokens before changing many individual widgets.
2. Prefer app-wide palette/QSS and a small number of reusable helper functions.
3. Modernize one workflow cluster at a time:
   - shell
   - creation flow
   - inspector docks
   - editing controls
4. Keep the visual system restrained and production-like.
5. Test the result on both dense and sparse screens.

## Technical Guidance
- Put shared theme resources in versioned files, not inline widget overrides everywhere.
- Avoid one-off styling unless it is clearly necessary.
- Preserve accessibility and focus states.
- Treat old iconography and spacing as part of the redesign, not as untouchable assets.

## Done Criteria
- A new visual layer exists outside of ad hoc widget tweaks.
- The updated workflow looks intentional and consistent across the affected screens.
