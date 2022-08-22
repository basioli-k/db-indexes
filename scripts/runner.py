from pathlib import Path
import argparse
import shutil
from example_generator import ExampleGenerator
from schema import Schema

def main():
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument("--schema", "-s", help="Schema path.", type=Path, required=True)
    arg_parser.add_argument("--horizontal", "-H", help="Path to horizonal db.", type=Path)
    arg_parser.add_argument("--vertical", "-v", help="Path to vertical db.", type=Path)
    arg_parser.add_argument("--dists", "-d", help="Path to distributions file.", type=Path, required=True)
    arg_parser.add_argument("--record-num", "-r", help="Number of records to generate.", type=int, required=True)
    arg_parser.add_argument("--primary-key", "-k", help="Index of column which will be a primary key.", type=int, required=True)
    
    args = arg_parser.parse_args()

    eg = ExampleGenerator(Schema(args.schema), args.dists.read_text().split(";"), args.primary_key)
    if not args.horizontal and not args.vertical:
        print("At least one database is required.")
        return

    if args.horizontal:
        args.horizontal.mkdir(exist_ok = True)
        shutil.copy(args.schema, args.horizontal)

    if args.vertical:
        args.vertical.mkdir(exist_ok = True)
        shutil.copy(args.schema, args.vertical)

    eg.write_records(args.horizontal, args.vertical, args.record_num)

if __name__ == "__main__":
    main()