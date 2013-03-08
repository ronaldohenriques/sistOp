// Aluno: Bruno E. O. Meneguele

#include <stdio.h>
#include <stdlib.h>
#include "tasklist.h"

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
  // Alocamento de 32kB de memoria para a pilha de 
  // contexto.
  stack = (char *) malloc(STACKSIZE);
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

  id_counter++;

  // Inicializando os atributos da tarefa.
  task->id = id_counter;
  task->priority = 0;
  task->age = 0;
#ifdef DEBUG
  printf("task_create: task %d created.\n", id_counter);
#endif
  makecontext(&(task->context), start_routine, 1, arg);
 
  // Todas as tarefas sao adicionas a lista, menos 
  // o dispatcher.
  if (task != &dispatcher) {
#ifdef DEBUG
    printf("task_create: list_append task %d\n", task->id);
#endif
    list_append(&ready_list, task);
  }

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

void task_yield()
{
  int err;

#ifdef DEBUG
  printf(">> task_yield()\n");
#endif

  // Cada tarefa que chama task_yield() eh colocada no 
  // fim da fila de tarefas prontas, menos o main().
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

  // Caso o dispatcher executar task_exit() o contexto
  // eh mudado para o main(). Caso for qualquer outra 
  // tarefa o contexto é mudado para o dispatcher.
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

int task_nice(int nice_level)
{
  int old_priority;

  old_priority = curr_task->priority;

  if (nice_level < abs(20)) {
#ifdef DEBUG
    printf("task_nice: old_priority = %d, nice_level = %d\n", 
      old_priority, nice_level); 
#endif
    curr_task->priority = nice_level;
  }

  return old_priority;
}

task_t* scheduler()
{
  task_t *task, *first, *aux;
  int higher = 20; // Armazena a maior prioridade da lista.

  // First aponta para o primeiro elemento da lista
  // de tarefas prontas.
  first = ready_list;

  // Task sera a tarefa analisada no momento.
  task = first;

  // Aux servira para armazenar a tarefa de maior prioridade
  // ateh o termino do scheduler.
  aux = first;

  do {
#ifdef DEBUG
    printf("scheduler: task: %d, priority: %d, age: %d\n", 
      task->id, task->priority, task->age);
#endif

    // Seleciona a tarefa com maior prioridade da lista de 
    // tarefas prontas.
    if ((task->priority - task->age) < higher) {
      higher = task->priority - task->age;
      aux = task;
    } 
    // Caso haver um empate entre as prioridades de duas 
    // tarefas a escolhida eh a que possuir maior prioridade
    // inicial (ignorando o task aging).
    else if ((task->priority - task->age) == higher) {
      if (task->priority < aux->priority)
        aux = task;
    }
  } while ((task = task->next) != first);

  // Realiza o task aging em todas as tarefas que nao 
  // que nao foram selecionadas.
  do {
    if (task == aux)
      continue;

    if ((task->priority - task->age) >= higher &&
      (task->priority - task->age) > -20)
      task->age++;
  } while ((task = task->next) != first);
  
  // Remove a tarefa da lista de tarefas prontas, 
  // pois esta estara em execucao.
  task = list_remove(&ready_list, aux);  
#ifdef DEBUG
    printf("scheduler: choosed task: %d, priority: %d, age: %d\n", 
      task->id, task->priority, task->age);
#endif

  // Zera a idade da tarefa escolhida.
  task->age = 0;

  // Retorna a tarefa que deve ocupar o processador.
  return task;
}

void dispatcher_body()
{
  int err;
  task_t* next; // Tarefa que ocupara o processador.

  // Caso haver alguma tarefa na lista de prontas
  // o while eh executado.
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

  // Finaliza o dispatcher, voltando para o main.
  task_exit(0);
}
