import subprocess
import sys
import struct
import os

cmake_source_dir=os.environ['CMAKE_SOURCE_DIR']
cmake_binary_dir=os.environ['CMAKE_BINARY_DIR']
cpp_compiler=os.environ['CMAKE_CXX_COMPILER']
arrp_install_dir=os.environ['ARRP_INSTALL_DIR']

arrp_exe = arrp_install_dir + '/bin/arrp'

def error(msg):
  sys.stderr.write('Error: ' + msg + '\n');
  return False

def info(msg):
  sys.stderr.write(msg + '\n');

def compile_arrp(source, output_name):
  info("Compiling Arrp...")
  subprocess.run([arrp_exe, '--target', 'generic', '--output', output_name],
                 input=source, universal_newlines=True, check=True)
  info("Compiling C++...")
  subprocess.run(
    [
      cpp_compiler,
      '-std=c++17',
      output_name + '-generic-main.cpp',
      '-I.',
      '-I' + arrp_install_dir + '/include',
      '-o', output_name
    ],
    check=True
  )

def compare(actual, expected):
  if actual != expected:
    return error("Expected '{}' but got '{}'.".format(expected, actual))
  return True

def to_byte_array(lst, format):
  buffer = bytearray();
  for item in lst:
    buffer += struct.pack(format, item)
  return buffer

def from_byte_array(buffer, format):
  lst = []
  offset = 0
  size = struct.calcsize(format)
  while offset < len(buffer):
    [item] = struct.unpack_from(format, buffer, offset)
    lst.append(item)
    offset += size
  return lst

# -------- TESTS ---------

def test_text_stream():
  source =  'input x : [~]int;    output y = x * 2;'
  compile_arrp(source, 'arrp-test')
  result = subprocess.run('./arrp-test', input='1 3 5', stdout=subprocess.PIPE, universal_newlines=True, check=True)
  info("Got output:\n" + result.stdout)
  return compare(result.stdout, '2\n6\n10\n')

def test_text_stream_noinput():
  source = 'output y = [i] -> i * 3;'
  compile_arrp(source, 'arrp-test')
  result = subprocess.run('./arrp-test | head -n 5', shell=True, stdout=subprocess.PIPE, universal_newlines=True, check=True)
  info("Got output:\n" + result.stdout)
  return compare(result.stdout, '0\n3\n6\n9\n12\n')

def test_text_scalar():
  source = 'input x : int; output y = x * 10;'
  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test'], input='7', stdout=subprocess.PIPE, universal_newlines=True, check=True)
  info("Got output:\n" + result.stdout)

  return compare(result.stdout, '70\n')

def test_input_literal_scalar():
  source = 'input x : int; output y = x * 10;'
  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test', 'x=3'], stdout=subprocess.PIPE, universal_newlines=True, check=True)
  info("Got output:\n" + result.stdout)

  return compare(result.stdout, '30\n')

def test_input_literal_array():
  source = 'input x : [4]int; output y = x * 10;'

  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test', 'x=1 3 5 7'], stdout=subprocess.PIPE, universal_newlines=True, check=True)

  info("Got output:\n" + result.stdout)

  if result.stdout != '10\n30\n50\n70\n':
    return error("Wrong output.")

  return True

def test_input_literal_stream():
  source = 'input x : [~]int; output y = x * 10;'

  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test', 'x=2 4 6'], stdout=subprocess.PIPE, universal_newlines=True, check=True)

  info("Got output:\n" + result.stdout)

  if result.stdout != '20\n40\n60\n':
    return error("Wrong output.")

  return True

def test_binary_stream_unbuffered():
  source = 'input x : [~]int; output y = x * 10;'

  compile_arrp(source, 'arrp-test')

  input = to_byte_array([1,3,5], '=i')
  info("Input: " + str(input))

  result = subprocess.run(['./arrp-test', '-b=0', '-f=raw'], input=input, stdout=subprocess.PIPE, check=True)
  info("Got output: " + str(from_byte_array(result.stdout,'=i')))

  expected_output = to_byte_array([10,30,50], '=i')

  return compare(result.stdout, expected_output)

def test_binary_stream_unbuffered_noinput():
  source = 'output y = [i] -> i * 2;'

  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test -b=0 -f=raw | head -c 20'], shell=True, stdout=subprocess.PIPE, check=True)
  info("Got output: " + str(from_byte_array(result.stdout,'=i')))

  expected_output = to_byte_array([i*2 for i in range(0,5)], '=i')

  return compare(result.stdout, expected_output)


def test_binary_stream_buffered():
  source = 'input x : [~]int; output y = x * 10;'

  compile_arrp(source, 'arrp-test')

  input = [i*2+1 for i in range(0,10)]
  raw_input = to_byte_array(input, '=i')
  info("Input: " + str(input))

  result = subprocess.run(['./arrp-test', '-b=4', '-f=raw'], input=raw_input, stdout=subprocess.PIPE, check=True)
  info("Got output: " + str(from_byte_array(result.stdout,'=i')))

  expected_output = [i*10 for i in input]
  expected_output = to_byte_array(expected_output, '=i')
  return compare(result.stdout, expected_output)

def test_binary_stream_buffered_noinput():
  source = 'output y = [i] -> i * 3;'

  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test -b=3 -f=raw | head -c 40'], shell=True, stdout=subprocess.PIPE, check=True)
  info("Got output: " + str(from_byte_array(result.stdout,'=i')))

  expected_output = [i*3 for i in range(0,10)]
  expected_output = to_byte_array(expected_output, '=i')
  return compare(result.stdout, expected_output)

def test_explicit_format():
  source = 'input x : [~]int; output y = x * 100;'

  compile_arrp(source, 'arrp-test')

  input = [3,5,7,9]
  raw_input = to_byte_array(input, '=i')
  info("Input: " + str(input))

  result = subprocess.run(['./arrp-test', 'x=pipe:raw', 'y=pipe:text'], input=raw_input, stdout=subprocess.PIPE, check=True)
  output = result.stdout.decode()
  info("Got output: " + output)

  expected_output = '300\n500\n700\n900\n'
  return compare(output, expected_output)

def test_file_text():
  source = 'input x : [~]int; output y = x * 10;'
  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test',
                           'x={}/test/exe/test-input'.format(cmake_source_dir),
                           'y=./test-output'],
                          stdout=subprocess.PIPE, check=True)
  f = open('./test-output', 'r')
  output = f.read()

  expected_output = '10\n30\n50\n20\n40\n60\n'

  return compare(output, expected_output)


def test_file_binary():
  source = 'input x : [~]int; output y = x;'
  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test',
                           'y=./test-output.raw:raw'],
                          input = '7 2 4 3', universal_newlines=True, check=True)

  out_file = open('./test-output.raw', 'rb')
  output = out_file.read()
  info("Checking output binary file:")
  if compare(output, to_byte_array([7,2,4,3], '=i')):
    info("OK")
  else:
    return False
  out_file.close()

  result = subprocess.run(['./arrp-test',
                           'x=./test-output.raw:raw'],
                          stdout=subprocess.PIPE, universal_newlines=True, check=True)

  expected_output = '7\n2\n4\n3\n'
  return compare(result.stdout, expected_output)


def test_file_binary_unbuffered():
  source = 'input x : [~]int; output y = x;'
  compile_arrp(source, 'arrp-test')

  result = subprocess.run(['./arrp-test',
                           '-b=0',
                           'y=./test-output.raw:raw'],
                          input = '7 2 4 3', universal_newlines=True, check=True)

  out_file = open('./test-output.raw', 'rb')
  output = out_file.read()
  info("Checking output binary file:")
  if compare(output, to_byte_array([7,2,4,3], '=i')):
    info("OK")
  else:
    return False
  out_file.close()

  result = subprocess.run(['./arrp-test',
                           '-b=0',
                           'x=./test-output.raw:raw'],
                          stdout=subprocess.PIPE, universal_newlines=True, check=True)

  expected_output = '7\n2\n4\n3\n'
  return compare(result.stdout, expected_output)


def test_file_text_binary_bool():
  source = 'input x : [~]bool; output y = x;'
  compile_arrp(source, 'arrp-test')

  # text input
  # unbuffered binary output
  result = subprocess.run(['./arrp-test',
                           '-b=0',
                           'y=./test-output.raw:raw'],
                          input = '1 0 1 1 0', universal_newlines=True, check=True)

  out_file = open('./test-output.raw', 'rb')
  output = out_file.read()
  info("Checking output binary file:")
  if compare(output, to_byte_array([1,0,1,1,0], '=B')):
    info("OK")
  else:
    return False
  out_file.close()

  # buffered binary input
  # text output
  result = subprocess.run(['./arrp-test',
                           '-b=3',
                           'x=./test-output.raw:raw'],
                          stdout=subprocess.PIPE, universal_newlines=True, check=True)

  expected_output = '1\n0\n1\n1\n0\n'
  return compare(result.stdout, expected_output)


def test_multi_input():
  source = 'input k : int; input x : [~]int; output y = x*k'
  compile_arrp(source, 'arrp-test')

  input = '0 3 6 9'

  result = subprocess.run(['./arrp-test', 'k=10'], input=input, stdout=subprocess.PIPE, universal_newlines=True, check=True)
  info("Got output: " + result.stdout)

  expected_output = '0\n30\n60\n90\n'
  return compare(result.stdout, expected_output)


def test_boolean_text_io():
  source =  'input x : [~]bool;    output y = not x;'
  compile_arrp(source, 'arrp-test')
  result = subprocess.run('./arrp-test', input='1 0 0 1 0 1', stdout=subprocess.PIPE, universal_newlines=True, check=True)
  info("Got output:\n" + result.stdout)
  return compare(result.stdout, '0\n1\n1\n0\n1\n0\n')


tests = {
    'text-stream': test_text_stream,
    'text-stream-noinput': test_text_stream_noinput,
    'text-scalar': test_text_scalar,
    'input-literal-scalar': test_input_literal_scalar,
    'input-literal-array': test_input_literal_array,
    'input-literal-stream': test_input_literal_stream,
    'binary-unbuffered-stream': test_binary_stream_unbuffered,
    'binary-unbuffered-stream-noinput': test_binary_stream_unbuffered_noinput,
    'binary-buffered-stream': test_binary_stream_buffered,
    'binary-buffered-stream-noinput': test_binary_stream_buffered_noinput,
    'explicit-format': test_explicit_format,
    'file-text': test_file_text,
    'file-binary': test_file_binary,
    'file-binary-unbuffered': test_file_binary_unbuffered,
    'file-text-binary-bool': test_file_text_binary_bool,
    'multi-input': test_multi_input,
    'boolean-text-io': test_boolean_text_io,
}

def main():
  test_name = None

  if len(sys.argv) >= 2:
    test_name = sys.argv[1]

  if test_name is None:
    error("Please provide a test name:")
    for key in tests.keys():
      print(key)
    return

  if not test_name in tests:
    error("Invalid test name: " + test_name)
    exit(1)

  test_func = tests[test_name]
  result = test_func()

  if result == True:
    info('OK')
  else:
    info('Failed')
    exit(1)

main()
