# Developper information

This page holds additional information for anyone wanting to dive into the code

## Build

To checkout doxygen-generated pages, run:  
`$ make dox`

An additional build switch can be used to generate .dot files, showing the graphs produced for algorithm X-cycles:  
`$ make GENDOT=Y`

These files will be stored in folder `out`.
For a given value, two graph dot files will be generated:
- `out/ls_V_X.dot`: holds strong-links only graph, for value V
- `out/la_V_X.dot`: holds graph completed with weak links


The graphs can after that be rendered as SVG files with makefile target (needs graphviz):  
`$ make dot`
