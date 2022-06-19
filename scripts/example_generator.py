import struct
import numpy as np
from pathlib import Path
from schema import Dist, Schema
import struct
import io

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

    def _increment_cnt(self, table_path):
        rec_count = (table_path / self.schema.table_name).with_suffix(".cnt")
        if not rec_count.exists():
            with rec_count.open("wb") as output:
                output.write(struct.pack("<I", 1))
        else:
            with rec_count.open("rb") as output:
                rec_num = struct.unpack("<I", output.read(np.uint32(0).itemsize))[0]
            with rec_count.open("wb") as output:
                rec_num += 1
                output.write(struct.pack("<I", rec_num))

    # appends to a table in a horizontal database
    def _append_to_hor(self, table_path : Path, record : tuple):
        table_name = (table_path / self.schema.table_name).with_suffix(".hor")
        
        if not table_name.exists():
            table_name.open("wb")

        with table_name.open("ab") as output:
            struct_format = "<" + "".join(np_to_struct[col] for col in self.schema.col_types)
            output.write(struct.pack(struct_format, *record))
        self._increment_cnt(table_path)

    def _append_to_ver(self, table_path : Path, record : tuple):
        table_dir = (table_path / self.schema.table_name)
        table_dir.mkdir(exist_ok=True)
        for i, col_name in enumerate(self.schema.col_names):
            column = (table_dir / col_name).with_suffix(".ver")
            with column.open("ab") as output:
                struct_format = "<" + np_to_struct[self.schema.col_types[i]]
                output.write(struct.pack(struct_format, record[i]))
        self._increment_cnt(table_path)

    def write_records(self, hor_table : Path, ver_table : Path, record_num : int):
        for _ in range(record_num):
            record = self._generate_record()
            self._append_to_hor(hor_table, record)
            self._append_to_ver(ver_table, record)