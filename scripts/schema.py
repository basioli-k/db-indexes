from pathlib import Path

class Schema:
    def __init__(self, schema_path : Path):
        self.table_name, layout = schema_path.read_text().split("\n")
        self.col_types = tuple( el.split(":")[0] for el in layout.split(";"))
        self.col_names = tuple( el.split(":")[1] for el in layout.split(";"))