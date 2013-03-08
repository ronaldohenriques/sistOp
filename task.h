#ifndef __TASK__
#define __TASK__

#include <ucontext.h>

typedef struct {
  int id;
  ucontext_t context;
} task_t;

void task_init();
int task_create(task_t *, void (*), void *);
int task_switch(task_t *);
void task_exit(int);

#endif
