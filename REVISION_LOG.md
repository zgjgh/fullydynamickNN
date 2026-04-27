# Revision Log

This file records paper-revision decisions that should remain stable across turns, so that later edits do not drift into local fixes that break global logic.

## Global constraints

1. **Do not casually edit the mathematical derivations.**
   Section 5 and Appendix A are tightly coupled. Revisions there should focus first on prose, structure, explanation, and terminology.

2. **Always re-read the whole active chapter before changing it.**
   Avoid local edits that break global narrative flow.

3. **Use `kNN` and `RkNN` more explicitly in search-optimization sections.**
   In Section 5 this is a search-algorithm optimization chapter, so the search terminology should remain visible.

4. **Avoid obvious “selling” language.**
   Do not write self-conscious contrasts like “this is not X but Y” or explicit authorial signaling that reads as over-packaging.

## Current active compile path

- Framework section currently compiles from:
  `Sections/4. Proposed Work_rewrite.tex`
- Fine-grained pruning section currently compiles from:
  `Sections/5. Fine-gained Pruning_1.tex`

## Current Section 5 priorities

### Priority 1: opening motivation chain

The first target is the opening part of Section 5:

- `Insufficient pruning at the non-leafnodes on \dualtree`
- `A step of point-level and low-dimensional distance based pruning`

This part currently contains the right ideas, but the prose is still too loose and repetitive.

The target structure should be:

1. non-leaf cluster pruning is conservative;
2. conservative pruning leaves false positives at leaf level;
3. therefore point-wise low-dimensional pruning is still valuable at leaf level;
4. once this new step is introduced, the main design question becomes how to choose its dimensionality.

### Priority 2: theorem framing in early Section 5

The theorem statements themselves should not be changed unless necessary, but their lead-in and follow-up prose should be tightened so readers can see:

- what theorem solves what subproblem;
- why the theorem appears at that exact place.

### Priority 3: strict vs simplified model

Later in Section 5 the prose should clearly distinguish:

- the strict mixed-distribution route;
- the simplified linear-log route.

This distinction is already present mathematically but not yet clean enough rhetorically.

### Priority 4: appendix relationship

Appendix A should be framed in the body text as supporting the local-Gaussian piece of the theory, not as if it re-proves the entire section.

## Current framework note

Framework has recently been re-stabilized:

- case-algorithm-explanation bridges were restored;
- stray Case 2 text was moved back into Case 2;
- over-packaged local-replacement lemma was removed again;
- `Dynamicizing \hdrtree` was rewritten to separate
  - why query-side dynamicization is needed, and
  - why the insertion rule minimizes the future RkNN pruning region.

Further framework edits should continue to respect the whole-chapter reading rule above.

## Section 5 restructuring notes

- The leaf-level low-dimensional pruning operation should be presented as an inserted search step, not as a lemma-level mathematical result.
- The direct expected-cost expression for a visited point should be presented as a derived model equation, not as a standalone lemma.
- The temporary inline heading `COST-AWARE DIMENSIONALITY SELECTION` was removed; the prose should instead flow directly from the pruning step to the dimensionality-selection problem.
- From the pruning-probability decomposition onward, Section 5 now uses a clearer three-part organization:
  1. `Rigorous pruning probability`
  2. `Simplified pruning probability`
  3. `Optimal pruning dimensionality`
- Inside the rigorous route, the two run-in guides are:
  - `Distance shrinkage.`
  - `Global distribution.`
  These should remain visually lighter than the three main blocks. The current style is:
  - main blocks: display-style bold line (`\noindent\textbf{...}\par`)
  - sub-guides: lighter italic run-ins (`\noindent\textit{...}`)
- In the `Global distribution` part, prose should first state the modeling goal, then explain why local distributions are sufficient, and only then introduce the mixture-model lemma.
- The lead-in to `Global distribution` should not pre-assert the mixed form. It should simply state that the global distribution is derived from the corresponding local distributions; the mixture form is then justified by the subsequent lemma.
- In Section 5, the four `PROPERTY` labels are formatted with double underlines rather than single underlines.
- The final simplified-optimum result should be presented as a prose conclusion plus a displayed formula, not as a separate lemma.
