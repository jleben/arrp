#! /usr/bin/env python3

import sys
import json
from pathlib import Path;
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('source')
parser.add_argument('report')
parser.add_argument('--program')
parser.add_argument('--program-options')
args = parser.parse_args()

# Parse report

def read_report(path):
    f = open(str(path), 'r')
    return json.load(f)

report = read_report(args.report)

# Parse source

source = open(args.source, 'r')

source_lines = [line[4:] for line in source if line.startswith('...?')]

if len(source_lines) == 0:
    exit(0)

type_str = source_lines[0]

def parse_type(text):
    text = text.rstrip().replace(' ', '');
    size = []
    is_stream = False
    if (text.startswith('[')):
        close_bracket = text.index(']',1)

        sizes_str = text[1:close_bracket]
        elem_type = text[close_bracket+1:]

        size_strs = sizes_str.split(',')
        for size_str in size_strs:
            if size_str == '~':
                is_stream = True
            else:
                size.append(int(size_str))
    else:
        elem_type = text

    return { 'is_stream': is_stream, 'dimensions': size, 'type': elem_type }

type_info = parse_type(type_str)

# Compare output type

if report['outputs'] is None:
    print("Program has no output.")
    exit(1)

output = report['outputs'][0]

if output['is_stream'] != type_info['is_stream']:
    if type_info['is_stream']:
        print("Stream expected.")
    else:
        print("Stream not expected.")
    exit(1)

def dimensions_str(dims):
    dims_str = ','.join([str(dim) for dim in dims])
    return '[' + dims_str + ']'


if not 'dimensions' in output:
    output['dimensions'] = []

if output['dimensions'] != type_info['dimensions']:
    print("Expected dimensions " +
          dimensions_str(type_info['dimensions']) +
          " but program has " +
          dimensions_str(output['dimensions']))
    exit(1)

if output['type'] != type_info['type']:
    print("Expected element type "  + type_info['type'] +
          " but program has " + output['type'])
    exit(1)

print("Type {} OK.".format(type_str))

# Compare output values

if args.program is None:
    exit(0)

values = []
for line in source_lines[1:]:
    elems = line.strip().replace(' ','').replace('(','').replace(')','').split(',')
    values += [float(elem) for elem in elems]

if len(values) == 0:
    exit(0)

cmd = args.program + ' -f text'
if args.program_options is not None:
    cmd += ' ' + args.program_options
cmd += ' | head -n ' + str(len(values))

result = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, universal_newlines=True)

out_lines = result.stdout.split('\n')
actual_values = [float(line.strip()) for line in out_lines if len(line.strip()) > 0]

if len(actual_values) < len(values):
    print("Expected at least " + str(len(values)) +
          " output elements but got only " + str(len(actual_values)))
    exit(1)

values_ok = True
for i in range(0,len(values)):
    if abs(values[i] - actual_values[i]) > 0.001:
        print("Output[{}] = {:.3f} (Error: Expected {:.3f}).".format(i, actual_values[i], values[i]))
        values_ok = False
    else:
        print("Output[{}] = {:.3f} (OK)".format(i, actual_values[i]))

if not values_ok:
    exit(1)
