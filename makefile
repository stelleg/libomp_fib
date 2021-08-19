all: fib_lomp fib_omp fib_ser

fib_ser.ll: fib_ser.c
	clang -S -emit-llvm -O1 $<

fib_omp.ll: fib_omp.c
	clang -fopenmp -S -emit-llvm -O1 $<

fib_lomp.ll: fib_lomp.c
	clang -g -S -emit-llvm -O1 $<

fib_%: fib_%.ll 
	clang -lomp $< -o $@
