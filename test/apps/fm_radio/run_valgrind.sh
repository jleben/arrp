
src="/home/jakob/code/stream-lang"

profile() {
  program_path=$1
  program_name=$(basename $program_path)
  echo "name = $program_name"
  valgrind --tool=callgrind --collect-atstart=no --dump-instr=yes --cache-sim=yes \
    --callgrind-out-file=$program_name.profile $program_path
}

profile $src/build/test/performance/fm_radio/fm_radio_s
profile $src/build/test/performance/fm_radio/fm_radio_c

