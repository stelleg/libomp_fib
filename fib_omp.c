#include<stdio.h>
#include<stdlib.h>

int serialfib(int n){
  return n <= 2 ? 1 : serialfib(n-1) + serialfib(n-2); 
}

int fib(int n){
  int x, y; 
  if(n < 35) return serialfib(n); 
  if(n <= 2) return 1; 
#pragma omp parallel
#pragma omp single 
{
#pragma omp task shared(x)
  x = fib(n-1); 
  y = fib(n-2); 
#pragma omp taskwait
}
  return x + y;
}


int main(int argc, char** argv){
  int n = argc > 1 ? atoi(argv[1]) : 40; 
  printf("%d\n", fib(n)); 
}

