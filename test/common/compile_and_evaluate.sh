
name="$1"
source="$2"
compile_options="$3"
run_options="$4"

program="$name"
report="$name.report.json"

set -e
set -x

"${CMAKE_BINARY_DIR}/arrp" "$source" --report "$report" -x "$program" --cpp-compiler-opts "-O0" ${compile_options}
"${CMAKE_SOURCE_DIR}/test/common/evaluate.py" "$source" "$report" --program "./$program" --program-options "${run_options}"
