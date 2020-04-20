
name="$1"
source="$2"
compile_options="$3"
run_options="$4"

report="$name.report.json"

set -e
set -x

"${ARRP_INSTALL_DIR}/bin/arrp" "$source" --interface stdio --report "$report" --output "$name" ${compile_options}
"${CXX}" -std=c++17 "$name-stdio-main.cpp" "-I." "-I${ARRP_INSTALL_DIR}/include" -o "$name"
"${CMAKE_SOURCE_DIR}/test/common/evaluate.py" "$source" "$report" --program "./$name" --program-options "${run_options}"
