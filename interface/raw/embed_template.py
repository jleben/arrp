import sys

template_path = sys.argv[1]
output_path = sys.argv[2]

template = open(template_path, 'r')

output = open(output_path, 'w')

output.write("#include <string>\n")
output.write('std::string arrp_raw_io_template = \n');

def to_c_string(text):
    return '"' + text.replace('"', '\\"') + '"';


for line in template:
    output.write(to_c_string(line.rstrip() + '\\n') + '\n');

output.write(";\n")
