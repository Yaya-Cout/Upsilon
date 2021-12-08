# This script generates two files:
# - A .rc: the list of the resources to be included in resources.rc
# - A C++ header: the mapping of the resource names and identifiers

import sys
import re
import argparse
import io

parser = argparse.ArgumentParser(description="Process some windows resources.")
parser.add_argument('assets', metavar='asset', type=str, nargs='+', help='The list of assets')
parser.add_argument('-o', required=True, help='The file to generate')
args = parser.parse_args()

def print_declaration(f, asset, identifier):
    f.write(str(identifier) + ' RCDATA ' + '"../assets/' + asset + '"\n')

def print_mapping(f, asset, identifier):
    f.write('ResourceID("' + asset + '", ' + str(identifier) + '),\n')

def print_mapping_header(f):
    f.write("#ifndef ION_SIMULATOR_WINDOWS_IMAGES_H\n")
    f.write("#define ION_SIMULATOR_WINDOWS_IMAGES_H\n\n")
    f.write("// This file is auto-generated by resgen.py\n\n")
    f.write("\nclass ResourceID {\n")
    f.write("public:\n")
    f.write("  constexpr ResourceID(const char * identifier, int id) : m_identifier(identifier), m_id(id) {}\n")
    f.write("  const char * identifier() const { return m_identifier; }\n")
    f.write("  int id() const { return m_id; }\n")
    f.write("private:\n")
    f.write("  const char * m_identifier;\n")
    f.write("  int m_id;\n")
    f.write("};\n\n")
    f.write("static constexpr ResourceID resourcesIdentifiers[] = {\n")

def print_mapping_footer(f):
    f.write("};\n\n")
    f.write("#endif\n")

def print(files, path, print_header, print_footer, process_asset):
    with open(path, "w") as f:
        print_header(f)
        for identifier, asset in enumerate(files, start=1000):
            process_asset(f, asset, identifier)
        print_footer(f)


if (args.o.endswith(".rc")):
    print(args.assets, args.o, lambda f: None, lambda f: None, print_declaration)

if (args.o.endswith(".h")):
    print(args.assets, args.o, print_mapping_header, print_mapping_footer, print_mapping)


