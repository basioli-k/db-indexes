from pathlib import Path
import numpy as np

cpp_to_np = {
    # "int8_t": np.int8,
    # "uint8_t": np.uint8,
    "int32_t": np.int32,
    "uint32_t": np.uint32,
    "int64_t": np.int64,
    "uint64_t": np.uint64,
    "float": np.float32,
    "double": np.float64
}

class Schema:
    def __init__(self, schema_path : Path):
        self.table_name, layout = schema_path.read_text().split("\n")
        self.col_types = tuple( cpp_to_np[el.split(":")[0]] for el in layout.split(";") )
        self.col_names = tuple( el.split(":")[1] for el in layout.split(";"))

    def row_size(self):
        return sum( np.dtype(ct).itemsize for ct in self.col_types)

    def get_col_size(self, dim):
        return np.dtype(self.col_types[dim]).itemsize

class Dist:
    def __init__(self, dist, typ):
        split_str = dist.split(":")
        self.type = split_str[0]
        self.params = tuple(typ(par) for par in split_str[1:])