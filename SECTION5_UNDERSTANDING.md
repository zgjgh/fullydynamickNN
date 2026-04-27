# Section 5 Understanding

This file records the current working understanding of `Sections/5. Fine-gained Pruning_1.tex` and the appendix material in `Sections/7. Conclusion.tex` that supports it. It is intended as a stable reference while revising the writing. It does **not** authorize changing the mathematical derivations themselves.

## 1. Core goal of Section 5

The section is not merely adding another pruning trick. Its real goal is:

- introduce a **leaf-level point-wise low-dimensional pruning step** for `kNN` and `RkNN` search on `\dualtree`; and
- derive a **cost-aware method for choosing the pruning dimensionality**.

The central optimization target is the pruning dimensionality `d`, not the pruning rule itself.

## 2. Why a new leaf-level pruning step is needed

The section first argues that pruning at non-leaf nodes is insufficient.

- Non-leaf pruning is cluster-level.
- A cluster may be retained because of its center-radius bound.
- Even when the cluster is retained, many individual points inside it can still be far enough to prune.

This is framed as a false-positive effect caused by cluster-level pruning.

The two examples make the same point in the two search settings:

- `kNN` search on the `\deltatree` side;
- `RkNN` search on the `\hdrtree` side.

So the new pruning step is motivated as a **leaf-level correction of residual false positives** left by non-leaf cluster pruning.

## 3. The new pruning step

The core theorem says:

- for a search-originating point `q` and a visited leaf-level point `v`,
- if the low-dimensional PCA distance `dist_PC(q,v)` already exceeds the pruning threshold `b_p`,
- then `v` can be pruned immediately.

Here:

- in `kNN` search, `b_p` is `dist_prune`;
- in `RkNN` search, `b_p` is `dist_kNN(v)`.

The correctness basis is the PCA contractive-distance property already established earlier:

- low-dimensional PCA distance never exceeds the full-dimensional distance,
- so a prune in low dimension is safe.

## 4. Why pruning dimensionality matters

The section then argues that the pruning dimensionality must be chosen carefully.

Two effects compete:

1. Using more PCA dimensions makes low-dimensional pruning stronger.
2. Using more PCA dimensions also makes each pruning distance computation more expensive.

This leads to the actual theoretical task:

- model the expected leaf-level computation cost as a function of `d`;
- find the `d` minimizing it.

## 5. The first probability theorem

The next theorem states that, for a point that should be pruned,

- if `a < b`,
- then pruning with `b` PCA dimensions succeeds with probability at least as high as pruning with `a` PCA dimensions.

This theorem does not yet solve the optimization problem. Its role is narrower:

- establish that larger `d` improves pruning success;
- justify the trade-off analysis that follows.

## 6. Cost model for one visited point

The section then defines a unit computation cost `c` for Euclidean distance evaluation and models the cost of processing one visited leaf-level point.

Three situations are considered:

1. The point should be retained.
   - Low-dimensional pruning cannot decide.
   - Full-dimensional distance is still needed.
   - Cost: `(D + d)c`.

2. The point should be pruned and is pruned by the low-dimensional step.
   - Cost: `dc`.

3. The point should be pruned but survives the low-dimensional step.
   - Full-dimensional distance is still needed.
   - Cost: `(D + d)c`.

From these cases, the section derives the main cost formula:

`E_cost(q,v)(d) = (D - D * P_p(d) + d)c`

This is the backbone of the theory. Once this formula is available, the whole remaining task becomes:

- characterize the pruning probability `P_p(d)`.

## 7. Key decomposition of the pruning probability

The section rewrites:

`P_p(d) = P(dist_PC(q,v) >= b_p)`

into:

`P(dist^2(q,v)/b_p^2 >= dist^2(q,v)/dist_PC^2(q,v))`

This is the main conceptual decomposition in the section.

It separates the problem into:

1. the random quantity `dist^2(q,v) / b_p^2`, whose distribution must be modeled; and
2. the shrinkage ratio `dist^2(q,v) / dist_PC^2(q,v)`, which depends on the pruning dimensionality.

So the pruning-probability problem becomes:

- one **distribution model**;
- one **dimensionality-shrinkage model**.

## 8. Distance shrinkage model

The section models the shrinkage ratio using PCA eigenvalues.

The logic is:

- the eigenvalue `e_i` associated with the `i`-th principal component represents variance retained along that component;
- therefore the information retained in the first `d` PCA dimensions can be approximated by
  `sum_{i=1}^d e_i / sum_{i=1}^D e_i`;
- hence
  `dist^2(q,v) / dist_PC^2(q,v)` is approximated by
  `sum_{i=1}^D e_i / sum_{i=1}^d e_i`.

Because the cumulative explained variance rises quickly at first and then flattens, the section models this relation by a logarithmic form:

`k ln(d) + b`

This is not presented as a strict theorem. It is a functional approximation used to make the later optimization tractable.

## 9. Stable phase and Condition 3

The next part introduces the stable-phase assumption and an additional Condition 3.

Its purpose is to make the distribution of `dist^2(q,v)/b_p^2` analyzable during a phase in which:

- data distribution changes slowly enough;
- the pruning-probability model is treated as effectively stable.

Condition 3 introduces:

- a partition of the ambient space into subspaces `O^i`;
- probabilities `P_i`;
- numbers of bound values or bound kernels `B_i`;
- weights `pi_ij`;
- local density models `f_ij`.

Its four properties formalize how:

- queries fall into subspaces;
- search processes encounter pruning bounds;
- and local conditional distributions are approximated.

This is the setup for the global-distribution model.

## 10. Local and global distribution

The section defines:

- a **local distribution**, corresponding to fixed subspace and bound-kernel conditions;
- a **global distribution**, corresponding to the overall distribution of `dist^2(q,v)/b_p^2`.

Using total probability and the stable-phase assumptions, the section shows that:

`f_{dist^2(q,v)/b_p^2} ≈ Σ_i Σ_j P_i pi_ij f_ij`

So the global distribution is modeled as a mixture of local distributions.

This is one of the main structural results of the section.

## 11. Why local distributions are approximately Normal

The section then proposes that each local distribution is approximately Normal:

`dist^2(q_{O^i}, v^j_{q_{O^i}}) / b_p^2(q_{O^i},j) ~ N(mu, sigma^2), approximately`

The full proof is deferred to Appendix A.

The role of this theorem is:

- allow each `f_ij` to be treated as approximately Gaussian;
- therefore make the global distribution a mixed Normal distribution.

Once this is accepted, the section obtains an explicit mixed-Gaussian form for the global density.

## 12. Parameter estimation

After giving the model, the section explains how to estimate its parameters at the beginning of a stable phase.

Two cases are handled separately:

### kNN-search case

- query point comes from `U`;
- visited point comes from `I`;
- `b_p` is the current farthest candidate in the kNN list.

Sampling collects:

- sampled `dist_kNN` values;
- the number of distinct `b_p` values encountered;
- the observed values of `dist^2(q,v)/b_p^2`.

From this, the section estimates:

- `M`;
- `B_i`;
- mixture weights and Gaussian parameters via EM.

### RkNN-search case

- query point comes from `I`;
- visited point comes from `U`;
- `b_p` is the visited point's `dist_kNN`.

Sampling again estimates:

- `M`;
- `B_i`;
- mixture parameters for the mixed Normal model.

So Appendix A.3 and the end of Section 5 give the operational route from theoretical model to usable parameters.

## 13. Strict form of the pruning probability

Once the shrinkage model and global distribution are both available, the section writes:

`P_p(d) ≈ ∫_{1/(k ln(d)+b)}^{+∞} mixed_normal_density(x) dx`

This is the strict model of the pruning probability.

Substituting it into the cost function yields a cost objective whose derivative gives a transcendental equation in `d`.

The section then proposes solving it numerically to obtain the optimal pruning dimensionality.

## 14. Simplified form of the pruning probability

The section then proposes a simplified model:

`P_p(d) ≈ k'' ln(d) + b''`

This is justified heuristically by:

- pruning probability should increase with the amount of PCA variance retained;
- retained variance itself is approximated by a logarithmic relation.

This simpler model leads to a closed-form optimum:

`d = D k''`

So the section offers two routes:

- a stricter mixed-distribution route;
- a simplified linear-log route.

## 15. What Appendix A is actually doing

Appendix A.1 and A.2 are not proving the whole section from scratch.

Their main purpose is narrower:

- provide a theoretical basis for why the local distribution can be treated as approximately Gaussian.

The proof chain is:

1. reduce the target quantity to a sum of squared PCA-coordinate differences;
2. center and normalize that sum;
3. apply characteristic-function bounds and Esseen inequality;
4. show convergence to the standard Normal distribution under the stated assumptions.

So Appendix A is mainly supporting the Gaussian local-distribution theorem, not the entire cost model.

## 16. Risk points to remember while revising the writing

When revising the prose around this section, the following must be respected:

1. **Do not touch the derivation chain casually.**
   The formulas and assumptions are tightly coupled.

2. **Keep the role of each part clear.**
   - false positives motivate the new pruning step;
   - the theorem establishes correctness;
   - the cost model defines the objective;
   - the decomposition of `P_p(d)` splits the problem;
   - the appendix supports the Gaussian local-distribution claim.

3. **Do not oversell the theory as a pure closed-form theorem system.**
   It is better understood as a cost-aware theoretical modeling framework with both strict and simplified versions.

4. **The writing burden is in the prose, not the math.**
   Most readers will not verify the appendix line by line. They must still be able to understand the main chain in the body text.

## 17. Immediate writing priority for Section 5

The first revision target should be the opening motivation chain:

- why non-leaf cluster pruning leaves false positives;
- why that motivates leaf-level point-wise pruning;
- why the real question is then how many PCA dimensions should be used.

If this opening chain becomes cleaner, the rest of the section will be much easier to revise without destabilizing the derivations.
