# db-indexes

### Schema files
First line of schema file is the table name.
Second line is a ; seperated list of : seperated entries representing columns in the form of <col_type>:<col_name>

### Dists
; separated distributions with their params, used in example_generator.py, number of dists should be the same as the number of columns.
supported
- uniform (ex.U:1,10)
- normal (ex.G:100,50)
