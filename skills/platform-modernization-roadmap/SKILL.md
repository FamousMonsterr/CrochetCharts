# Platform Modernization Roadmap

## Purpose
Expand the desktop product into online and Android forms without a destructive rewrite.

## Strategic Rule
Do not build web and Android directly on top of desktop widget code. Extract domain and document logic first, then expose it through shared services and APIs.

## Required Outputs
- staged roadmap
- shared-core boundary
- server/API boundary
- mobile delivery recommendation
- billing/auth integration points

## Workflow
1. Identify reusable core concerns:
   - chart document model
   - templates
   - transforms and layout operations
   - serialization
2. Separate UI orchestration from domain behavior.
3. Define online capabilities:
   - auth
   - subscription state
   - document storage
   - collaboration policy
4. Choose the first Android strategy that minimizes duplicate product logic.
5. Keep the desktop app shipping while platform work progresses.

## Business Constraints
- monthly, quarterly, annual plans
- annual plan target is `5000 RUB`
- annual must be `40%` cheaper than twelve monthly payments

## Done Criteria
- The roadmap is implementable in stages.
- Desktop, web, and Android responsibilities are not conflated.
