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
Please note that biasing sampling of a specific node of the tree is a different problem.

#### Global Sampling Strategy

In oder to bias sampling of the subtree starting at node $n_i$
by a factor of $x$,
the weight corresponding to $n_i$ and all its descendants nodes need to be multiplied by a foactor of $x$ in the roulette distribution.
This is an $\mathcal{O}(n)$ time operation.

#### Hierachichal Sampling Strategy

In oder to bias sampling of the subtree starting at $node $n_i$
by a factor of $x$,
the roulette distributions of the ancestors of the subtree (all nodes leading to $n_i$) need to be updated to account
for the bias by:
 - multiplying the wheight of the subtree leading to $n_i$ by a factor of $x$ in $n_i$'s parent,
 - updating the the weight of the subtrees leading to $n_i$ by taking into account the subtree udated weight.
This is a
$\mathcal{O}(k \log_k n)$
time operation on average,
where $k$ is the mean arity of the tree.

#### Biasing of a node

Biasing sampling of a specific node is also possible, and requires updating a single weight in the global sampling strategy
(still an $\mathcal{O}(n)$ time operation).

The hierarchical sampling strategy is a bit more complex, but can be achieved if the biases of subtrees are stored on the node at the base of the subtree. the complexity is the same as biasing a subtree, $\mathcal{O}(k \log_k n)$.

## The Dynamic Tree Case

Lets consider a tree, potentially infinite, where only a subset of nodes are known.
We add the constraint that the arity of known nodes is also known and is noted $a_i$ for node $n_i$.
We consider the reachable nodes to be the union of all known nodes plus the unknown direct children of
known nodes, and the cardinality of this ensemble to be $n$.

Exploring an unknown node $n_j$ yields its arity $a_j$ and thus increases the number of reachable nodes to
$n + a_j$.

### Uniform Sampling

Uniform sampling through the reachable nodes can be done using both strategy given above.
Nonetheless the cost of updating the different sampling strategy, as new nodes are explored is not the same.
The global sampling strategy would require traversing the tree every time a new node becomes known and it's arity discovered this is an $\mathcal{O}(n)$.
A contrario, the hierarchical sampling strategy allows updating the roulette ditributions using a similar algorithm used to bias a subtree, and thus is an $\mathcal{O}(k \log_k n)$ operation.

### Biased sampling

Biased sampling works the same way as in the static case for both strategy and incur the same cost.
