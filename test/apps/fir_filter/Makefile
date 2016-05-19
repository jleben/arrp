streamc=~/code/stream-lang/build/compiler/streamc
input=~/code/stream-lang/test/performance/input
drivers=~/code/stream-lang/test/performance/drivers
icc=~/intel/bin/icc

all: test2_gnu test2_icc test2_icc.s


test2_gnu: test2_gnu.s
	g++ -std=c++11 test2_gnu.s -o test2_gnu -lpthread -lpapi

test2_gnu.s: test2_cpp $(drivers)/test2_driver.cpp
	g++ -std=c++11 -O2  -Winline $(drivers)/test2_driver.cpp -I. -S -o test2_gnu.s



test2_icc: test2_icc.o
	$(icc) test2_icc.o -o test2_icc -lpthread -lpapi

test2_icc.o: test2_cpp $(drivers)/test2_driver.cpp
	$(icc) -std=c++11 -O3  -inline-forceinline -Winline \
	$(drivers)/test2_driver.cpp -I. -c -o test2_icc.o


test2_icc.s: test2_cpp $(drivers)/test2_driver.cpp
	$(icc) -std=c++11 -O3 -inline-forceinline -Winline $(drivers)/test2_driver.cpp -I. -S -o test2_icc.s



test2_cpp: test2_opt.cpp test2_raw.cpp

test2_opt.cpp:
	$(streamc) $(input)/test2.in --cpp test2_opt --split-stmts --reverse filter_s1 1

test2_raw.cpp: $(input)/test2.in
	$(streamc) $(input)/test2.in --cpp test2_raw --no-opt-schedule --split-stmts
