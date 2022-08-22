import struct
import numpy as np
from pathlib import Path
from schema import Dist, Schema
import struct
from tqdm import tqdm

np_to_struct = {
    np.int8: "b", # used to fill the block with zeros if needed
    # np.uint8: "B",
    np.int32: "i",
    np.uint32: "I",
    np.int64: "q",
    np.uint64: "Q",
    np.float32: "f",
    np.float64: "d" 
}

BLOCK_SIZE = 512 # bytes

class ExampleGenerator:
    def __init__(self, schema : Schema, dists : list, primary_key : int):
        self.schema = schema
        self.dists = tuple(Dist(dist, self.schema.col_types[i]) for i, dist in enumerate(dists))  #distributions
        self.hor_block = BLOCK_SIZE
        self.ver_block = list(BLOCK_SIZE for _ in self.schema.col_names)
        self.pk = primary_key
        self.keys = set()

    # TODO add distributions
    def _generate_nums(self, index : int):
        typ = self.schema.col_types[index]
        params = self.dists[index].params

        if self.dists[index].type == "U":
            return typ(np.random.uniform(*params))
        elif self.dists[index].type == "G":
            return typ(np.random.normal(*params))
        else:
            raise ValueError("Unsupported distribution. Distribution", self.dists[index], "was given")

    def _generate_records(self, num):
        records = []
        for _ in range(num):
            record = []
            for i in range(len(self.schema.col_names)):
                num = self._generate_nums(i)
                if i == self.pk:
                    if num in self.keys:
                        while num in self.keys:
                            num = self._generate_nums(i)
                    self.keys.add(num)
                record.append(num)

            records.append(tuple(record))
        
        return records

    def _increment_cnt(self, table_path, rec_len):
        rec_count = (table_path / self.schema.table_name).with_suffix(".cnt")
        if not rec_count.exists():
            with rec_count.open("wb") as output:
                output.write(struct.pack("<I", rec_len))
        else:
            with rec_count.open("rb") as output:
                rec_num = struct.unpack("<I", output.read(np.uint32(0).itemsize))[0]
            with rec_count.open("wb") as output:
                rec_num += rec_len
                output.write(struct.pack("<I", rec_num))

    # appends to a table in a horizontal database
    def _append_to_hor(self, table_path : Path, records : list):
        table_name = (table_path / self.schema.table_name).with_suffix(".hor")
        
        if not table_name.exists():
            table_name.open("wb")

        with table_name.open("ab") as output:
            struct_format = "<" + "".join(np_to_struct[col] for col in self.schema.col_types)
            rsize = self.schema.row_size()
            for record in records:
                if self.hor_block < rsize:
                    if self.hor_block:
                        output.write(struct.pack("<" + "".join(np_to_struct[np.int8] for _ in range(self.hor_block)), 
                            *(np.int8(0) for _ in range(self.hor_block))))
                    self.hor_block = BLOCK_SIZE

                output.write(struct.pack(struct_format, *record))
                self.hor_block -= rsize

        self._increment_cnt(table_path, len(records))

    def _append_to_ver(self, table_path : Path, records : list):
        table_dir = (table_path / self.schema.table_name)
        table_dir.mkdir(exist_ok=True)
        for i, col_name in enumerate(self.schema.col_names):
            column = (table_dir / col_name).with_suffix(".ver")
            with column.open("ab") as output:
                struct_format = "<" + np_to_struct[self.schema.col_types[i]]
                col_size = self.schema.get_col_size(i)
                for record in records:
                    if self.ver_block[i] >= col_size:
                        output.write(struct.pack(struct_format, record[i]))
                        self.ver_block[i] -= col_size
                    else:
                        if self.ver_block[i]:
                            raise Exception("Column size should always divide the block size.")
                            # output.write(struct.pack("<" + "".join(np_to_struct[np.int8] for _ in range(self.self.ver_block[i])), 
                            #     *(np.int8(0) for _ in range(self.self.ver_block[i]))))
                        self.ver_block[i] = BLOCK_SIZE
                        output.write(struct.pack(struct_format, record[i]))

        self._increment_cnt(table_path, len(records))

    def write_records(self, hor_table : Path, ver_table : Path, record_num : int):
        rsize = self.schema.row_size()
        batch_size = 10**8 // rsize     # approx no. of records in 100 MB
        while tqdm(record_num):
            records = self._generate_records(min(batch_size, record_num))
            record_num = max(0, record_num - batch_size)
            if hor_table:
                self._append_to_hor(hor_table, records)
            if ver_table:
                self._append_to_ver(ver_table, records)