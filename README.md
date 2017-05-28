# <div align = center>gSpan</div>

**gSpan** is an algorithm for mining frequent subgraphs.

## Data format

### short syntax description

Input data is a one or many _input graphs_.
Output data is one or many _patterns_ with _mappings_ to _input graph_.
\# is comment line


```
#
# begin section with input graph (or transaction)
#
t <graph_id>
v <vertex_id> <value>
...[repeat v]...
e <edge_id> <vertex_id> <vertex_id> <value>
...[repeat e]...

...[repeat t]..


#
# begin section with mined pattern, it is also graph
#
p <pattern_id>
v <vertex_id> <value>
...[repeat v]...
e <edge_id> <vertex_id> <vertex_id> <value>
...[repeat e]...

#
# each pattern has multiple mappings to original input graph
#
m <mapping_id>
v <vertex_id> <graph_id> <vertex_id> # map pattern vertex to original vertex
...[repeat v]..
e <edge_id> <graph_id> <edge_id>     # map pattern edge to original edge
...[repeat e]...

...[repeat m]...

...[repeat p]...

```


### Reference
- [Paper](http://www.cs.ucsb.edu/~xyan/papers/gSpan-short.pdf)

gSpan: Graph-Based Substructure Pattern Mining, by X. Yan and J. Han. 
Proc. 2002 of Int. Conf. on Data Mining (ICDM'02). 

