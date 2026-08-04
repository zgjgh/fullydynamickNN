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

5. **Introduce contributions in active voice.**
   At the first mention of a contribution, give it a separate sentence with the authors as the subject (for example, ``We propose ...''). Do not bury its first appearance in a sentence mainly describing other work or procedural order.

## Current active compile path

- VLDBJ working sections currently compile from:
  `Sections_VLDBJ_Working/`
- Framework section currently compiles from:
  `Sections_VLDBJ_Working/4. Proposed Work_rewrite_1.tex`
- Fine-grained pruning section currently compiles from:
  `Sections_VLDBJ_Working/5. Fine-gained Pruning_1.tex`

## 2026-08-05: Align HDR maintenance prose with Algorithms 1--3

- Introduced the proposed direct-link mechanism independently in the Section 4.1 roadmap.
- Clarified that each query points to its hosting leaf and each non-root HDR-Tree node points to its parent.
- Replaced the unsupported ``structural repartitioning events'' wording: routine updates retain existing centers, while overflow growth accounts for k-means construction of a new subtree.
- Rewrote query deletion and pruning-parameter refresh in algorithm order, with line-level hooks to Algorithms 2 and 3.
- Moved the with/without-link cost comparisons to the deletion and refresh operations that use the links.
- Corrected the deletion description so cluster summaries, rather than the query point itself, are updated, and treated fanout $f$ as fixed.
- Recast the direct links in active voice, distinguished initialization from update-time maintenance, and removed the link-space analysis.
- Reordered the deletion explanation into routine path maintenance followed by the exceptional subtree collapse.
- Refined the deletion and refresh time analyses to emphasize eliminated path localization and PCA-distance work without overstating the asymptotic effect of fixed fanout.

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
- Section 5 received a prose-only B-class cleanup pass on 2026-04-27 covering PROPERTY 2-3, the `Distance shrinkage` explanation, the statement texts of the two remaining lemmas, and the simplified-pruning paragraph, without changing formulas or derivation logic.
- In Appendix A.3, the standard EM update formulas and their tutorial-style explanation were removed to save space; A.3 now cites EM as a standard Gaussian-mixture parameter-estimation routine and keeps the focus on the paper-specific sampling and grouping logic.
- In Appendix A.3, a shared two-stage estimation pipeline was made explicit before the kNN/RkNN split. The kNN and RkNN subsections now keep only their case-specific sampling sources and collected statistics, while preserving the original sampling steps, grouping rules, and estimation logic.

- Replaced Appendix A.3 in active Sections/7. Conclusion.tex with the A.3 text from the '(2)' project version, leaving A.1 and A.2 unchanged, so further A.3 edits will now start from that variant.
- After replacing A.3 from the '(2)' project version, a single mojibake line introduced in Appendix A.2 was repaired without changing any A.3 content.
- In Appendix A.3, the EM-algorithm exposition was compressed again on top of the current `(2)`-style A.3: the kNN subsection now cites standard EM without reproducing update equations, and the RkNN subsection now mirrors that concise treatment while preserving the original sampling and grouping logic.
- In Appendix A.3, visible-body terminology in the active `Conclusion.tex` was normalized from mixed `kNN Search` / `RkNN Search` forms to consistent `kNN search` / `RkNN search` wording, without changing any sampling logic, formulas, or case structure.
- In Appendix A.3, the opening lead-in and the two scene-definition sentences for the kNN and RkNN cases were rewritten for grammar and clarity only, while keeping the original roles of `q`, `v`, `b_p`, and `\mathcal{O}` unchanged.
- In Appendix A.3, the kNN and RkNN scene-definition sentences now explicitly cite the original exact search procedures on \deltatree and \hdrtree, respectively, without changing any local estimation logic.
- In Appendix A.3, the old-prose phrases `The first work / The second work`, the kNN-case rationale paragraph, and the rough RkNN-case transitions (`Similar to ...`, `the below relationship should hold that`) were rewritten for cleaner journal prose without changing any sampling steps, grouping rules, or formulas.

## 2026-08-04: explicit dynamic HDR-Tree maintenance

- Split the opening of `Sections_VLDBJ_Working/4. Proposed Work_rewrite_1.tex` into two paragraphs: the first establishes the dynamically maintainable dual-index components, and the second states that the chapter coordinates maintenance of the materialized kNN join table and dual-index state under all four update types.
- Reframed the HDR-Tree extension around two update effects: structural changes caused by updates to $U$, and query-dependent pruning-value changes caused by updates to $I$.
- Replaced the previously stored per-query ancestor path with a direct query-to-leaf pointer plus per-node parent links.
- Made `HDR_Adjust` an explicit bottom-up procedure: recompute the leaf maximum from resident queries, recompute each ancestor maximum from immediate child summaries, and stop a threshold refresh once the propagated maximum is unchanged.
- Added the path-local correctness argument and the $O(\theta+Lf)$ threshold-refresh bound, while explicitly excluding structural radius maintenance from that bound and avoiding any unsupported whole-operation comparison with the original HDR-Tree implementation.
- Reorganized the extension into three corresponding operations: query insertion, query deletion, and pruning-parameter refresh after reference updates. The subsection opening now states this order explicitly before the three run-in modules appear.
- Clarified both gaps left by the original HDR-Tree setting: it does not define structural updates when the indexed query set changes, and its reference-update treatment only sketches recursive adjustment of cluster maximum kNN distances.
- Added a separate query-deletion algorithm that follows the query-to-leaf pointer upward, restores the cluster parameters, and collapses a non-leaf subtree into a full-dimensional leaf when its remaining population falls below the construction threshold $\theta$.
- Clarified that insertion levels created beyond the anticipated PCA schedule continue with dimensionality $D$ after the schedule reaches the full dimensionality.
- Replaced the combined mode-based adjustment routine with distinct `Update_HDR_Delete` and `Update_HDR_Refresh` calls and synchronized the four fully dynamic update cases accordingly.
- Replaced the unexplained phrase `two directions` with the explicit search directions: kNN search from $U$ to $I$, and RkNN search from an updated reference in $I$ back to affected queries in $U$.
- Split the description of the two original HDR-Tree limitations into short sentences and used `structural updates` for query-side membership and topology changes.
- Moved the query-to-leaf pointer and node-to-parent link into a separate `Direct update links` module shared by insertion, deletion, and pruning-parameter refresh.
- Restated the over-depth rule directly from the original HDR-Tree construction: after the estimated height $\widehat{L}=\lceil\log_f|U|\rceil$ is exceeded, each additional level uses $d_l=D$.
- Placed the three HDR update algorithms as bottom floats so that their shared link definition and operation-specific prose are read before the pseudocode.
- Moved the `Direct update links` module immediately after query insertion: insertion initializes the query-to-leaf and node-to-parent links, and the following deletion and pruning-parameter refresh procedures consume them.
- Added explicit cost bounds for all six framework algorithms. The three new HDR procedures are analyzed using tree height $L$, fanout $f$, leaf threshold $\theta$, and level dimensionalities $d_l$; the four join-update cases use compositional kNN, RkNN, row-update, and index-update costs.
- Quantified the link benefit without overstating it: path-directed lookup drops from up to $O(Lf)$ child-cluster tests to $O(L)$ parent-link traversal, an improvement of up to a factor of $f$ for path localization. Leaf-summary and ancestor-summary recomputation remain part of the complete update cost.
- Shortened the reference-insertion pseudocode replacement line so that no algorithm statement wraps in the compiled two-column layout.
- Compressed Section 4.1 so that the HDR-Tree extension remains subordinate to the main dual-maintenance framework: removed the detailed reconstruction of the original estimated-height/PCA schedule, retained only the full-dimensional rule beyond the estimated height, and folded the standalone locality/cost discussion into the relevant operations.
- Made the Section 4.1 roadmap structurally complete by naming query insertion, the link augmentation introduced in this work, query deletion, and pruning-parameter refresh in their presentation order.
- Strengthened ownership of the link contribution while keeping its claim bounded: the text now states that our extension adds query-to-leaf pointers and node-to-parent links to replace recursive path recovery, with the factor-$f$ benefit applying specifically to path localization.
- Rebalanced the subsection lengths after compression: Section 4.1 is now shorter than Section 4.2, while all three HDR update complexities remain attached concisely to their corresponding algorithms.
- Repositioned the three HDR algorithms at column tops so that Algorithms 1--3 all appear before the Section 4.2 heading without introducing a float-barrier dependency or leaving a large blank region.
- Refined the direct-link complexity claim: child inspection decreases from $O(Lf)$ to $O(L)$ by up to a factor of $f$, while the $O(f\sum_l d_l)$ geometric path-search work is eliminated rather than folded into the same factor. The text now emphasizes that, without the links, this path lookup is repeated for every affected query during a reference update.
- Unified the per-level PCA dimensionality notation from $d(l)$ to $d_l$ between Background and Section 4.
- Made the radius-maintenance contract explicit so that the insertion and deletion bounds are independently auditable: centers remain fixed between repartitioning events, leaf radii scan at most $\theta$ resident queries, and internal radii are maintained as valid enclosing upper bounds from at most $f$ child summaries.
