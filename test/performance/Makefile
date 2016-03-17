streamc=~/code/stream-lang/build/compiler/streamc
input=~/code/stream-lang/test/performance/input
drivers=~/code/stream-lang/test/performance/drivers
icc=~/intel/bin/icc

all: test2_gnu test2_icc

test2_gnu: test2_cpp $(drivers)/test2_driver.cpp
	g++ -std=c++11 -O3 $(drivers)/test2_driver.cpp -I. -o test2_gnu

test2_icc: test2_cpp $(drivers)/test2_driver.cpp
	$(icc) -std=c++11 -O3 $(drivers)/test2_driver.cpp -I. -o test2_icc

test2_cpp: test2_opt.cpp test2_raw.cpp

test2_opt.cpp:
	$(streamc) $(input)/test2.in --cpp test2_opt --split-stmts --reverse filter_s1 1

test2_raw.cpp: $(input)/test2.in
	$(streamc) $(input)/test2.in --cpp test2_raw --split-stmts --no-opt-schedule
