// Aluno: Bruno E. O. Meneguele

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

#include "tasklist.h"
//#include "semaphore.h"

#define STACKSIZE 32768 // 32kB

// Flag de preempcao.
int PREEMPTION = 1;

// Semaforos.
unsigned int sem_id_counter = 0;

// Timer.
int t0 = 0;
int t1 = 0;
int ticks = 20;
unsigned int sys_clk = 0;
struct itimerval timer;
struct sigaction action;

// Tasks.
unsigned int id_counter = 0;
task_t main_task, dispatcher; 
task_t *curr_task, *old_task;
task_t *ready_list, *sleep_list;

int systime() 
{
  return sys_clk;
}

void enable_preemption(int value)
{
  PREEMPTION = value;
}

void timer_handler(int signum) 
{
  //printf("sys_clk++\n");
  sys_clk++;

  if (PREEMPTION == 1) {
    if (--ticks == 0)
      task_yield();
  }
}

// -------- TASKS --------
void task_init()
{
  int err;

#ifdef DEBUG
  printf(">> task_init()\n");
#endif
  setvbuf(stdout, 0, _IONBF, 0);
  
  // --------- Timer --------
  action.sa_handler = timer_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;

  if (sigaction(SIGALRM, &action, 0) < 0)
  {
    perror("sigaction error: ");
    exit(1);
  }

  timer.it_value.tv_usec = 1;
  timer.it_value.tv_sec = 0;
  timer.it_interval.tv_usec = 1000;
  timer.it_interval.tv_sec = 0;

  if (setitimer(ITIMER_REAL, &timer, 0) < 0)
  {
    perror("setitimer error: ");
    exit(1);
  }
  // ------ Fim Timer -------  

  main_task.id = id_counter;
  main_task.priority = 0;
  main_task.birth_time = systime();
  main_task.proc_time = 0;
  main_task.activations = 0;
  main_task.wakeup_time = -1;
  main_task.join_wkp = NULL;
  main_task.exit_code = -2;
  err = getcontext(&(main_task.context));

  if (err == -1) {
    perror("task_init: main getcontext failed.\n");
    return;
  }

  curr_task = &main_task;

#ifdef DEBUG
  printf("task_init: task main created.\n");
#endif
 
  err = task_create(&dispatcher, dispatcher_body, 0);

  if (err == -1) {
    perror("task_init: dispatcher task_create failed.\n");
    return;
  }

  list_append(&ready_list, &main_task);
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
  task->birth_time = systime();
  task->proc_time = 0;
  task->activations = 0;
  task->exit_code = -2;
  task->wakeup_time = -1;
  task->join_wkp = NULL;

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

  t1 = systime();
  curr_task->proc_time += t1 - t0;

  if (task == &dispatcher)
    enable_preemption(0);
  else
    enable_preemption(1);

  curr_task->activations++;
  old_task = curr_task;
  curr_task = task;
  t0 = systime();
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
  // fim da fila de tarefas prontas.
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
  int err, exec_time, proc_time, num_act;
  task_t *task;

#ifdef DEBUG
  printf(">> task_exit()\n");
#endif

  curr_task->exit_code = exit_code;
  curr_task->death_time = systime();
  exec_time = curr_task->death_time - curr_task->birth_time;
  proc_time = curr_task->proc_time;
  num_act = curr_task->activations;
  
  printf("Task %d exit: execution time %d ms, processor time %d ms, ",
    curr_task->id, exec_time, proc_time);
  printf("%d activations\n", num_act);

  enable_preemption(0);
  
  while (list_size(curr_task->join_wkp) > 0) {
    task = list_remove(&(curr_task->join_wkp), curr_task->join_wkp);
    list_append(&ready_list, task);
  }

  enable_preemption(1);

  if (curr_task != &dispatcher)
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

  if (abs(nice_level) < 20) {
#ifdef DEBUG
    printf("task_nice: old_priority = %d, nice_level = %d\n", 
        old_priority, nice_level); 
#endif
    curr_task->priority = nice_level;
  }

  return old_priority;
}

int task_join(task_t *task)
{
  enable_preemption(0);

  if (task == NULL) {
    enable_preemption(1);
    return -1;
  } else if (task->exit_code == 0) {
    enable_preemption(1);
    return task->exit_code;
  } else {
    list_remove(&ready_list, curr_task);
    list_append(&(task->join_wkp), curr_task);
    task_switch(&dispatcher);
    enable_preemption(1);
    return task->exit_code;
  }

  return 0;
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
  // foram selecionadas.
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
  task_t *next; // Tarefa que ocupara o processador.
  task_t *task, *aux;

  // Caso haver alguma tarefa na lista de prontas ou adormecidas
  // o while eh executado.
  while ((list_size(ready_list) > 0) || (list_size(sleep_list) > 0)) {
    task = sleep_list;

    // Acorda tarefas que estao na fila de adormecidos
    // por causa de algum task_sleep(t);
    do {
      if (sleep_list == NULL)
        break;

      /*
         printf("systime: %d, id: %d, wakeup: %d\n", sys_clk, task->id,
         task->wakeup_time);
       */

      if ((task->wakeup_time > 0) && (task->wakeup_time <= systime())) {
        aux = task;
        sleep_list = sleep_list->next;
        task = sleep_list;

        list_remove(&sleep_list, aux);
        list_append(&ready_list, aux);
      }

    } while ((task = task->next) != sleep_list);

    if (list_size(ready_list) > 0) {
      next = scheduler();

      if (next) {
        ticks = 20;
        err = task_switch(next); 

        if (err == -1) {
          perror("dispatcher_body: task_switch failed.\n");
          return;
        }
      }
    }
  }

  // Finaliza o dispatcher, voltando para o main.
  task_exit(0);
}

void task_sleep(int t)
{
  if (t > 0) {
    enable_preemption(0);
    list_remove(&ready_list, curr_task);
    list_append(&sleep_list, curr_task);
    curr_task->wakeup_time = systime() + t*1000;
    task_switch(&dispatcher);
  }

  task_yield();
}
// ----- END TASKS --------

// ----- SEMAPHORES -------
int sem_create(semaphore_t *s, int value)
{
  if (s == NULL)
    return -1;
  
  s->id = sem_id_counter++;
  s->queue = NULL;
  s->count = value;
  s->status = ENABLED;
  return 0;
}

int sem_down(semaphore_t *s)
{
  enable_preemption(0);

  if (s == NULL || s->status == DESTROYED) {
    enable_preemption(1);
    return -1;
  }
  
  if (s->count <= 0) {
    list_remove(&ready_list, curr_task);
    list_append(&(s->queue), curr_task);
    s->count--;
    task_switch(&dispatcher);
  } else {
    s->count--;
  }
  
  enable_preemption(1);
  return 0;
}

int sem_up(semaphore_t *s)
{
  task_t *task;

  enable_preemption(0);
  
  if (s == NULL || s->status == DESTROYED) {
    enable_preemption(1);
    return -1;
  }

  s->count++;

  if (s->count <= 0) {
    task = list_remove(&(s->queue), s->queue);
    list_append(&ready_list, task);
  }

  enable_preemption(1);
  return 0;
}

int sem_destroy(semaphore_t *s)
{
  task_t *task;

  enable_preemption(0);

  if (s == NULL) {
    enable_preemption(1);
    return -1;
  }

  while (list_size(s->queue) > 0) {
    task = list_remove(&(s->queue), s->queue);
    list_append(&ready_list, task);
  }

  s->status = DESTROYED;
  enable_preemption(1);
  return 0;
}


// --- END SEMAPHORES -----

// ------- MQUEUE ---------

int mqueue_create(mqueue_t *queue, int max, int size) {
  if (queue == NULL)
    return -1;
  
  queue->queue = malloc(max * size);
  queue->max = max;
  queue->size = size;
  queue->count = 0;
  queue->write_idx = 0;
  queue->read_idx = 0;

  sem_create(&queue->s_free, max);
  sem_create(&queue->s_buffer, 1);
  sem_create(&queue->s_msg, 0);

  return 0;
}

int mqueue_send(mqueue_t *queue, void *msg) 
{
  if (queue == NULL)
    return -1;
  
  if (sem_down(&queue->s_free) == -1)
    return -1;
  if (sem_down(&queue->s_buffer) == -1)
    return -1;

  memcpy(&(queue->queue[queue->write_idx * queue->size]), msg, queue->size);
  queue->write_idx = (queue->write_idx + 1) % queue->max;
  
  if (sem_up(&queue->s_buffer) == -1)
    return -1;
  if (sem_up(&queue->s_msg) == -1)
    return -1;

  return 0;
}

int mqueue_recv(mqueue_t *queue, void *msg) 
{
  if (queue == NULL)
    return -1;
  
  if (sem_down(&queue->s_msg) == -1)
    return -1;
  if (sem_down(&queue->s_buffer) == -1)
    return -1;
  
  memcpy(msg, &(queue->queue[queue->read_idx * queue->size]), queue->size);
  queue->read_idx = (queue->read_idx + 1) % queue->max;
  
  if (sem_up(&queue->s_buffer) == -1)
    return -1;
  if (sem_up(&queue->s_free) == -1)
    return -1;

  return 0;
}

int mqueue_destroy(mqueue_t *queue)
{
  if (queue == NULL)
    return -1;

  if (sem_destroy(&queue->s_free) == -1)
    return -1;
  if (sem_destroy(&queue->s_buffer) == -1)
    return -1;
  if (sem_destroy(&queue->s_msg) == -1)
    return -1;

  // Zerando conteudo
  memset(queue->queue, '0', queue->size*queue->max);

  return 0;

}

int mqueue_msgs(mqueue_t *queue)
{
  if (queue == NULL)
    return -1;

  return (&queue->s_msg)->count;
}

// ----- END MQUEU --------

// ------ FIIIM :D --------
