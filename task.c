#include "task.h"
#include <stdio.h>
#include <stdlib.h>

#define STACKSIZE 32768 // 32kB

unsigned int id_counter = 0;
task_t main_task, *curr_task, *old_task;

void task_init()
{
  int err;
#ifdef DEBUG
  printf(">> task_init()\n");
#endif
  setvbuf(stdout, 0, _IONBF, 0);
  err = getcontext(&(main_task.context));

  if (err == -1) {
    perror("task_init: getcontext failed.\n");
    return;
  }

  main_task.id = id_counter;
  curr_task = &main_task;
#ifdef DEBUG
  printf("task_init: task main created.\n");
  printf("<< task_init()\n");
#endif
}

int task_create(task_t *task, void (*start_routine), void *arg)
{
  char *stack = NULL;
  int err;

#ifdef DEBUG
  printf(">> task_create()\n");
#endif
  stack = (char *) malloc(STACKSIZE);
  id_counter++;
  err = getcontext(&(task->context));

  if (err == -1) {
    perror("task_create: getcontext failed.\n");
    return err;
  } 

  if (stack != NULL) {
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
  } else {
#ifdef DEBUG
    perror("task_create: stack malloc failed.\n");
#endif
    return -1;
  }

  task->id = id_counter;
#ifdef DEBUG
  printf("task_create: task %d created.\n", id_counter);
#endif
  makecontext(&(task->context), start_routine, 1, arg);
#ifdef DEBUG
  printf("<< task_create()\n");
#endif
  
  return id_counter;
}
 
int task_switch(task_t *task)
{
  int err;

#ifdef DEBUG
  printf(">> task_swtich()\n");
#endif
  
  old_task = curr_task;
  curr_task = task;
  err = swapcontext(&(old_task->context), &(task->context));

  if (err == -1) {
    perror("task_switch: swapcontext failed.\n");
    return err;
  }

#ifdef DEBUG
  printf("<< task_swtich()\n");
#endif

  return 0;
}

void task_exit(int exit_code)
{
  int err;

#ifdef DEBUG
  printf(">> task_exit()\n");
#endif
  err = task_switch(&(main_task));

  if (err == -1)
    perror("task_exit: task_switch failed.\n");

#ifdef DEBUG
  printf(">> task_exit()\n");
#endif
}
