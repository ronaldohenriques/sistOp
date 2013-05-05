// Aluno: Bruno E. O. Meneguele

#ifndef __TASK__
#define __TASK__

#include <ucontext.h>

// Estrutura usada para cada tarefa.
typedef struct task_t {
  struct task_t *prev;  // Apontador para a tarefa anterior da lista.
  struct task_t *next;  // Apontador para a proxima tarefa da lista.
  struct task_t *join_wkp;  // Task que sera aguardada (task_join)
  int id;               // Identifiador da tarefa.
  int priority;         // Prioridade da tarefa, manipulada no scheduler.
  int age;              // Idade da tarefa, usada no 'task aging' do scheduler.
  ucontext_t context;   // Contexto da tarefa.
  int birth_time;       // Tempo de nascimento.
  int death_time;       // Tempo de morte/encerramento.
  int proc_time;        // Tempo de processamento.
  int activations;      // Numero de ativacoes.
  int exit_code;        // Codigo de encerramento
  int wakeup_time;      // Tempo em que deve acordar.
} task_t;

typedef struct semaphore_t {
  int id;
  task_t *queue;
  int count;
  int status;
} semaphore_t;

typedef struct mqueue_t {
  char *queue;
  semaphore_t s_free;
  semaphore_t s_buffer;
  semaphore_t s_msg;
  int max;
  int size;
  int count;
  int write_idx;
  int read_idx;
} mqueue_t;

enum { 
  ENABLED = 0,
  DESTROYED = 1,
};

int systime();

void enable_preemption(int);

void timer_handler(int);

// Inicializacao da tarefa main e de variaveis.
void task_init();

void task_sleep(int);

// Criacao da tarefa, atribuindo todos valores necessarios 
// aos atributos da estrutura.
// RETORNO: id da tarefa caso sucesso, -1 caso erro.
int task_create(task_t *, void (*), void *);

// Realiza a troca de contexto entre a task atual e a task 
// solicitada.
// RETORNO: 0 caso sucesso, -1 caso erro.
int task_switch(task_t *);

// Libera o uso do processador para o dispatcher ativar
// outra tarefa.
void task_yield();

// Encerra a tarefa atual.
void task_exit(int);

// Retorna o valor da tarefa atual.
int task_id();

// Funcao que o dispetcher usara para ativar a tarefa selecionada
// pelo scheduler().
void dispatcher_body();

// Decisao politica entre qual tarefa ira usar o processador quando 
// o mesmo estiver livre.
// RETORNO: tarefa selecionada para usar o processador.
task_t* scheduler();

// Atribui uma prioridade a tarefa atual.
// RETORNO: prioridade da tarefa antes da nova.
int task_nice(int);

int task_join(task_t *);

int sem_create(semaphore_t *, int);

int sem_down(semaphore_t *);

int sem_up(semaphore_t *);

int sem_destroy(semaphore_t *);

int mqueue_create(mqueue_t *, int, int);

int mqueue_send(mqueue_t *, void *);

int mqueue_recv(mqueue_t *, void *);

int mqueue_destroy(mqueue_t *);

int mqueue_msgs(mqueue_t *);

#endif
