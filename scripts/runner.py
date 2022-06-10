from pathlib import Path
import argparse
from example_generator import ExampleGenerator
from schema import Schema

def main():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--schema", "-s", help="Schema path.", type=Path)
    arg_parser.add_argument("--dists", "-d", help="Path to distributions file.", type=Path)
    arg_parser.add_argument("--table-path", "-t", help="Path to table folder.", type=Path)
    arg_parser.add_argument("--record-num", "-r", help="Number of records to generate.", type=int)
    
    args = arg_parser.parse_args()

    eg = ExampleGenerator(Schema(args.schema), args.dists.read_text().split(";"))
    eg.write_records(args.table_path, args.record_num)

if __name__ == "__main__":
    main()