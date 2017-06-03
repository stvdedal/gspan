# <div align = center>gSpan</div>

**gSpan** is an algorithm for mining frequent subgraphs.

## Library

Header-only in include directory

## Example command

```
$ ./example/gspan --help
Usage: gspan [options]
Graph-based substructure pattern mining.
Depending on the graph count in input, there are two modes:
  1. input contains one graph. Mined patterns belong to this one;
       in this case only --mincount=NUM option is used
  2. input contains many graphs. Mined patterns belong to some graph in input;
       in this case --minsupp=NUM option is used, as more useful.
Options:
  -i, --input FILE        file to read, default stdin
  -o, --output FILE       file to write, default stdout
  -c, --mincount NUM      minimal count, integer value, default 1
  -s, --minsupp NUM       minimal support, 0..1
  -l, --legacy            use tgf format for input and output (slower!)
  -e, --embeddings [opts] none, autgrp, all. default is none
  -h, --help              this help

```

### Test

[Performace Test](example/test/TEST.md)

### Data format

two formats supported:

#### First format (recomodated)

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
p <pattern_id> # occurence: <num>
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

#### Legacy format

```

#
# input
#
t # <graph_id>
v <vertex_id> <value>
e <vertex_id> <vertex_id> <value>

#
# output
#
t # <pattern_id> * <support>
parent : <pattern_id>
v <vertex_id> <value>
e <vertex_id> <vertex_id> <value>
x: <graph_id> ...

```

### Reference
- [Paper](http://www.cs.ucsb.edu/~xyan/papers/gSpan-short.pdf)

gSpan: Graph-Based Substructure Pattern Mining, by X. Yan and J. Han. 
Proc. 2002 of Int. Conf. on Data Mining (ICDM'02). 

