import struct
import numpy as np
from pathlib import Path
from schema import Dist, Schema
import struct

np_to_struct = {
    # np.int8: "b", # for now use only types of size that is a multiple of 4
    # np.uint8: "B",
    np.int32: "i",
    np.uint32: "I",
    np.int64: "q",
    np.uint64: "Q",
    np.float32: "f",
    np.float64: "d" 
}

class ExampleGenerator:
    def __init__(self, schema : Schema, dists : list):
        self.schema = schema
        self.dists = tuple(Dist(dist, self.schema.col_types[i]) for i, dist in enumerate(dists))  #distributions

    # TODO add distributions
    def _generate_num(self, index : int):
        typ = self.schema.col_types[index]
        params = self.dists[index].params

        if self.dists[index].type == "U":
            return typ(np.random.uniform(*params))
        elif self.dists[index].type == "G":
            return typ(np.random.normal(*params))
        else:
            raise ValueError("Unsupported distribution. Distribution", self.dists[index], "was given")

    def _generate_record(self):
        record = tuple(self._generate_num(i) for i in range(len(self.schema.col_names)))
        return record

    # appends to a table in a horizontal database
    def _append_to_hor(self, table_path : Path, record : tuple):
        struct_format = "<" + "".join(np_to_struct[col] for col in self.schema.col_types)
        table_name = (table_path / self.schema.table_name).with_suffix(".hor")
        if not table_name.exists():
            table_name.open("w")    #create empty file

        with table_name.open("ab") as output:
            output.write(struct.pack(struct_format, *record))

    # TODO write this function
    # appends to a table in a vertical database
    def _append_to_ver(self, table_name : Path, record : tuple):
        pass

    def write_records(self, table_path : Path, record_num : int):
        for _ in range(record_num):
            record = self._generate_record()
            self._append_to_hor(table_path, record)
            self._append_to_ver(table_path, record)