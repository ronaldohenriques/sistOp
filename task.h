#ifndef __TASK__
#define __TASK__

#include <ucontext.h>

typedef struct task_t {
  struct task_t *prev;
  struct task_t *next;
  int id;
  ucontext_t context;
} task_t;

void task_init();
int task_create(task_t *, void (*), void *);
int task_switch(task_t *);
void task_yield();
void task_exit(int);
int task_id();
void dispatcher_body();
task_t* scheduler();

#endif
