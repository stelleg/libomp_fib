all: fib

fib.ll: fib.cpp
	clang++ -S -emit-llvm -O1 $<

fib: fib.ll
	clang++ -lomp fib.ll -o fib
