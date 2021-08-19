#include <stdio.h>
#include <omp.h>

// detached untied
#define PTASK_FLAG_DETACHABLE 0x40

int started = 0; 

// Compiler-generated code (emulation)
typedef struct ident {
  void* dummy; // not used in the library
} ident_t;

typedef struct {
  int type;
  union {
    void *task;
  } ed;
} kmp_event_t;

typedef struct shar { // shareds used in the task
  int n; 
  int x;
} *pshareds;

typedef struct task {
  pshareds shareds;
  int(*routine)(int,struct task*);
  int part_id;
  omp_event_handle_t evt;
} *ptask, kmp_task_t;

typedef void(*fork_entry_t)(int*, int*, ...);
typedef void(*task_entry_t)(int, ptask);

#ifdef __cplusplus
extern "C" {
#endif

extern int __kmpc_global_thread_num(void *id_ref);
extern ptask __kmpc_omp_task_alloc(ident_t *loc, int gtid, int flags,
                                   size_t sz, size_t shar, task_entry_t rtn);

extern omp_event_handle_t __kmpc_task_allow_completion_event(
                              ident_t *loc_ref, int gtid, ptask task);

extern void __kmpc_fork_call(ident_t *, int nargs, 
                       fork_entry_t microtask, ...);
                
extern int __kmpc_omp_task(ident_t *loc_ref, int gtid, ptask task);                 
extern int __kmpc_omp_taskwait(ident_t *loc_ref, int gtid);

extern int __kmpc_single(ident_t *loc_ref, int gtid);
extern int __kmpc_end_single(ident_t *loc_ref, int gtid);
#if __cplusplus
}
#endif

int fib(int n);

// User's code, outlined into task entry
void task_entry(int gtid, ptask parenttask) {
  int n = parenttask->shareds->n;  
  parenttask->shareds->x = fib(n); 
}

void fork_entry(int *gtid, int *btid, int *n, int *x, int *y){
  if(*n <= 2){
    *x = 1;
    *y = 0; 
    return;
  }

  // Now that we've entered a parellel context, ensure only one thread
  // continues with the following code
  __kmpc_single(NULL, *gtid);
  ptask task;
  pshareds psh;
  
  // Create task thunk (delayed computation)
  task = (ptask)__kmpc_omp_task_alloc(NULL, *gtid, PTASK_FLAG_DETACHABLE, sizeof(struct task), sizeof(struct shar), &task_entry);
  psh = task->shareds;
  psh->n = *n-1; 
  
  // Submit thunk to runtime
  __kmpc_omp_task(NULL, *gtid, task);

  // Compute the second call while 
  *y = fib(*n-2);
  
  // Wait for the (n-1) call to finish
  __kmpc_omp_taskwait(NULL, *gtid);

  // Copy the result over
  *x = task->shareds->x; 
  __kmpc_end_single(NULL, *gtid); 
  //__kmpc_end_single(NULL, *gtid); 
 
}

int fib(int n) {
  int x, y;
  __kmpc_fork_call(NULL, 3, (fork_entry_t)&fork_entry, &n, &x, &y);
  return x + y;   
}

int main(int argc, char **argv) {
  int n = argc > 1 ? atoi(argv[1]) : 10;
  printf("%d\n", fib(n));
  return 0;
}
