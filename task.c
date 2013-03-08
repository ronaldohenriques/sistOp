#include "tasklist.h"
#include <stdio.h>
#include <stdlib.h>

#define STACKSIZE 32768 // 32kB

unsigned int id_counter = 0;
task_t main_task, dispatcher, 
  *curr_task, *old_task, *ready_list;

void task_init()
{
  int err;
#ifdef DEBUG
  printf(">> task_init()\n");
#endif
  setvbuf(stdout, 0, _IONBF, 0);
  err = getcontext(&(main_task.context));

  if (err == -1) {
    perror("task_init: main getcontext failed.\n");
    return;
  }

  main_task.id = id_counter;
  curr_task = &main_task;
#ifdef DEBUG
  printf("task_init: task main created.\n");
#endif
 
  if (err == -1) {
    perror("task_init: dispatcher getcontext failed.\n");
    return;
  }
  
  err = task_create(&dispatcher, dispatcher_body, 0);

  if (err == -1) {
    perror("task_init: dispatcher task_create failed.\n");
    return;
  }

#ifdef DEBUG
  printf("task_init: task dispatcher created.\n");
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
  
  if ((task != &main_task) || (task != &dispatcher))
    list_append(&ready_list, task);

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

void task_yield()
{
  int err;

#ifdef DEBUG
  printf(">> task_yield()\n");
#endif
  if (curr_task != &main_task)
    list_append(&ready_list, curr_task);
  
  err = task_switch(&dispatcher);

  if (err == -1) {
    perror("task_yield: dispatcher switch failed.\n");
    return;
  }

#ifdef DEBUG
  printf("<< task_yield()\n");
#endif
}

void task_exit(int exit_code)
{
  int err;

#ifdef DEBUG
  printf(">> task_exit()\n");
#endif

  if (curr_task == &dispatcher)
    err = task_switch(&main_task);
  else
    err = task_switch(&dispatcher);

  if (err == -1) {
    perror("task_exit: task_switch failed.\n");
    return;
  }

#ifdef DEBUG
  printf(">> task_exit()\n");
#endif
}

int task_id()
{
  return curr_task->id;
}

task_t* scheduler()
{
  task_t *task;

  task = list_remove(&ready_list, ready_list);  
  return task;
}

void dispatcher_body()
{
  int err;
  task_t* next;

  while (list_size(ready_list) > 0) {
    next = scheduler();

    if (next) {
      err = task_switch(next); 

      if (err == -1) {
        perror("dispatcher_body: task_switch failed.\n");
        return;
      }
    }
  }

  task_exit(0);
}
