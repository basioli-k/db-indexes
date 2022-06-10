from multiprocessing.sharedctypes import Value
import numpy as np
from pathlib import Path
from schema import Schema

cpp_to_np = {
    "int8_t": np.int8,
    "uint8_t": np.uint8,
    "int32_t": np.int32,
    "uint32_t": np.uint32,
    "int64_t": np.int64,
    "uint64_t": np.uint64,
    "float": np.float32,
    "double": np.float64
}

class ExampleGenerator:
    def __init__(self, schema : Schema, dists : list):
        self.schema = schema
        self.dists = tuple(dists)  #distributions

    def _resolve_params(self, typ):
        if issubclass(typ, np.floating):
            return np.finfo(typ).min, np.finfo(typ).max
        elif issubclass(typ, np.integer):
            return np.iinfo(typ).min, np.iinfo(typ).max
        else:
            raise ValueError("Unsupported type", typ)

    # TODO support floats and add distributions
    def _generate_num(self, index : int):
        typ = cpp_to_np[self.schema.col_types[index]]
        params = self._resolve_params(typ)

        if self.dists[index] == "U":
            return typ(np.random.uniform(*params))
        else:
            raise ValueError("Unsupported distribution.")
        # elif self.dists[index] == "G":
        #     pass

    def _generate_record(self):
        record = tuple(self._generate_num(i) for i in range(len(self.schema.col_names)))
        return record

    # TODO write these functions (use struct pack)
    # appends to a horizontal database
    def _write_to_hor(self, table_name : Path, record : tuple):
        pass

    # appends to a vertical database
    def _write_to_ver(self, table_name : Path, record : tuple):
        pass

    def write_records(self, table_path : Path, record_num : int):
        for _ in range(record_num):
            record = self._generate_record()
            print(record)   # TODO remove
            self._write_to_hor(table_path, record)
            self._write_to_ver(table_path, record)