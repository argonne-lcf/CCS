# Tree Sampling Notes and Considerations

## The Static Tree

The static tree case is the case where the whole tree of $n$ nodes is known and accessible from the beginning.

### Uniform Sampling

The uniform sampling problem consisting in selecting a random node (and position) in the tree with uniform probability.

#### Global Sampling Strategy

Uniformely sampling a static tree is straightforward, with
$\mathcal{O}(n)$
space and
$\mathcal{O}(\log n)$
time, requiring 1 random sample. Gather the nodes in an array and create a roulette distribution with uniform weights
$\left(\begin{array}[b]{r}w_1 & \dots & w_n \end{array}\right)$
to sample the index space.
Both operations are $\mathcal{O}(n)$ in time.
TODO: insert ref.

#### Hierachichal sampling strategy

Sampling can be done while descending through the tree, where each child subtree
is sampled with a probability proportional to it's subtree weight, and the current
node with a probability proportional to 1. The array of probability for a node with
an arity of $m$
where the weight of a child subtree at location
$i$
is given by
$cw_i$
is the vector:
$\left(\begin{array}[b]{r}cw_1 & \dots & cw_m & 1 \end{array}\right)$
The overhead of sampling is still
$\mathcal{O}(n)$
space, but the time overhead depends on the tree geometry and is
$\mathcal{O}(n)$
in the worst case (linear tree) and
$\mathcal{O}(\log_2 n)$ in the best (flat tree)
with an average
$\mathcal{O}(k \log_k n)$
where k is the mean arity of the nodes.
$\mathcal{O}(\log_k n)$ random samples are required (but optimizations exist to only use 1).

### Biased sampling

The biased sampling problem consist in sampling some subtrees with increased probability with respect to the rest of the tree.

#### Global Sampling Strategy

In oder to bias sampling of the subtree starting at node $n_i$
by a factor of $x$,
the weight corresponding to $n_i$ and all its descendants nodes need to be multiplied by a foactor of $x$ in the roulette distribution.
This is an $\mathcal{O}(n)$ time operation.

#### Hierachichal sampling strategy

In oder to bias sampling of the subtree starting at $node $n_i$
by a factor of $x$,
the roulette distributions of the ancestors of the subtree (all nodes leading to $n_i$) need to be updated to account
for the bias by multiplying the wheight of the subtrees leading to $n_i$ by a factor of $x$.
This is a
$\mathcal{O}(k \log_k n)$
time operation on average,
where $k$ is the mean arity of the tree.
