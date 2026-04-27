# Packaging Tree

This note records the paper-positioning tree for the manuscript. It is the control
outline for later revisions of the introduction, background, related work,
framework, pruning, and experiments sections.

## 0. Global Positioning

- This paper is not about inventing a new standalone search index.
- This paper is about solving: exact + fully dynamic + high-dimensional + kNN join maintenance.
- The primary sell is a bidirectional maintenance framework, not a mechanical combination of two trees.

## 1. Problem Layer

- Object: a materialized kNN join table.
- Task: maintain the join result while both `U` and `I` change.
- Constraints:
  - exact
  - high-dimensional
  - fully dynamic
- Core interpretation: this is not one-shot query processing; it is continuous maintenance of a neighborhood view.

## 2. Difficulty Layer

- High dimensionality:
  - full-distance evaluation is expensive
  - pruning becomes weaker
- Two-sided updates:
  - updates to `U` require forward kNN repair
  - updates to `I` require reverse affected-query localization
- Exactness:
  - the maintained result must match recomputation
  - in the fully dynamic setting, approximation errors can propagate through both maintenance paths

## 3. Core Insight Layer

- Fully dynamic exact maintenance naturally decomposes into two complementary primitives:
  - forward repair
  - reverse localization
- A single-sided index only accelerates one of them.
- Therefore the right abstraction is a bidirectional locate-and-repair framework.

## 4. Structural-Requirement Layer

- The framework is not satisfied by any exact index.
- In the exact high-dimensional setting, the underlying search structures must jointly provide:
  - effective reduced-space pruning
  - hierarchical clustering / localization
  - exact verification at the leaf level
  - summaries that can be updated and synchronized during maintenance
  - one component aligned with forward repair
  - one component aligned with reverse localization
- This requirement layer is what turns the paper from "two old trees" into a principled design.

## 5. Solution Layer

- Name: `Dual-Tree`
- Essence:
  - a bidirectional locate-and-repair framework
  - instantiated with a dual-index architecture
- Components:
  - `Delta-Tree` on `I`: forward kNN search and repair
  - `HDR-Tree` on `U`: reverse localization
- Correct narrative:
  - first identify the abstract dual-path maintenance structure
  - then identify the structural requirements imposed by exact high-dimensional maintenance
  - then instantiate the framework with `Delta-Tree` and a dynamicized `HDR-Tree`
- The key is not simply "two trees".
- The key is:
  - what is done at which update stage
  - which repair path uses which structure
  - how the join table and index summaries remain synchronized

## 6. HDR-Tree Update Contribution

- Do not sell this as a standalone breakthrough.
- Do not claim a brand-new HDR-Tree.
- Correct positioning:
  - an enabling mechanism inside Contribution 1
  - it closes the fully dynamic maintenance loop
- Standard framing:
  - adapt HDR-Tree from a static-query reverse index
  - into a mutable reverse-localization component
  - and synchronize its summaries with join-table repair

## 7. Pruning Layer

- Secondary mechanism: point-wise pruning at leaf level
- Main technical sell: a cost-aware analytical framework for choosing pruning dimensionality
- Proper emphasis:
  - first, pruning is pushed from cluster level to point level
  - more importantly, the paper derives a principled dimension-selection model
- Do not oversell point-wise pruning as the biggest contribution.

## 8. Exact vs Approximate Layer

- Mention approximate methods once and with restraint.
- Correct line:
  - ANN is suitable for approximate retrieval
  - this paper studies exact dynamic maintenance
  - approximation changes the contract
- Correct exactness framing:
  - not only more faithful neighborhood semantics under the chosen metric
  - but also a stronger correctness guarantee for maintenance
- Avoid:
  - claiming ANN usually finds only one neighbor
  - claiming exact kNN always captures true similarity better
  - claiming exact is universally preferable in practice

## 9. Application Layer

- Prefer high-precision, high-cost, high-stakes scenarios:
  - healthcare / clinical databases
  - financial monitoring / fraud / risk
  - scientific or engineering pipelines
- Avoid relying on recommendation or embedding-serving narratives.

## 10. Contribution Layer

- Contribution 1:
  - a bidirectional locate-and-repair framework
  - instantiated by dual index + HDR dynamicization + synchronization
- Contribution 2:
  - point-wise pruning and cost-aware modeling
- Contribution 3:
  - comprehensive evaluation
  - one or two strong numbers are acceptable

## 11. Section Responsibilities

- Introduction:
  - problem, constraints, contract, and framework-level positioning
- Background:
  - definitions
  - PCA lower-bound fact
  - interface-level explanation of Delta-Tree and HDR-Tree
  - why they satisfy the structural requirements of exact high-dimensional maintenance
- Related Work:
  - what static and one-sided dynamic methods do
  - what they do not provide
  - why other exact methods and prior join methods do not solve fully dynamic bidirectional maintenance
- Framework:
  - formally present the bidirectional locate-and-repair cycle
  - show how `Delta-Tree + dynamicized HDR-Tree` instantiate it
- Pruning:
  - point-wise mechanism + analytical model

## 12. Preferred One-Sentence Thesis

We study exact fully dynamic high-dimensional kNN join as a bidirectional maintenance problem over a materialized neighborhood view, identify the structural requirements imposed by exact high-dimensional maintenance, and instantiate the resulting dual-path framework with `Delta-Tree`, a dynamicized `HDR-Tree`, and cost-aware fine-grained pruning.

## 13. Red Lines

- Do not write the paper as if it invented two new trees.
- Do not over-isolate HDR-Tree update as a heavyweight standalone contribution.
- Do not present exactness as semantic truth.
- Do not oversell the framework as fully index-agnostic or arbitrarily plug-and-play.
- Do not let chapters drift away from the maintenance-view interpretation.
- Always return to:
  - materialized join
  - two repair primitives
  - structural requirements
  - coordinated maintenance
