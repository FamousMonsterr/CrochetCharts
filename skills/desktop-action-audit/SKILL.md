# Desktop Action Audit

## Purpose
Audit every desktop action so the team can say what each control is supposed to do, what it actually does, and whether the behavior is trustworthy.

## Inputs
- `src/mainwindow.*`
- `src/crochettab.*`
- `src/scene.*`
- `src/crochetchartcommands.*`
- `src/*dock*.*`
- relevant `.ui` files

## Workflow
1. Enumerate user-facing entry points:
   - menu actions
   - toolbar actions
   - dock buttons
   - mode switches
   - context-sensitive controls
2. Trace the signal/slot path into implementation.
3. Mark each action:
   - correct
   - suspicious
   - broken
   - unverified
4. Record any hidden coupling:
   - selected items required
   - active tab required
   - active scene required
   - specific guideline mode required
5. Produce manual verification steps for the riskiest actions.

## Output Format
- feature name
- entry point
- implementation path
- expected behavior
- current behavior
- risk level
- next action

## Done Criteria
- No major toolbar or dock is left unclassified.
- The highest-risk actions have a concrete reproduction path.
