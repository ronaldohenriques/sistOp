#include <stdio.h>
#include <stdlib.h>

#include "task.h"

int buffer[5];
int prod_idx = 0;
int cons_idx = 0;
semaphore_t s_buffer, s_item, s_vaga;
task_t p1, p2, p3, c1, c2;

void producer(char *name)
{
  int item;

  while (1) {
    task_sleep(1);
    item = random() % 100;

    sem_down(&s_vaga);
    sem_down(&s_buffer);
    buffer[prod_idx] = item;
    prod_idx = (prod_idx + 1) % 5;
    printf("%s produziu %d\n", name, item);
    sem_up(&s_buffer);
    sem_up(&s_item);
  }
}

void consumer(char *name)
{
  int item;

  while (1) {
    sem_down(&s_item);
    sem_down(&s_buffer);
    item = buffer[cons_idx];
    cons_idx = (cons_idx + 1) % 5;
    printf("\t\t%s consumiu %d\n", name, item);
    sem_up(&s_buffer);
    sem_up(&s_vaga);

    task_sleep(1);
  }
}

int main(void) 
{
  task_init();

  sem_create(&s_buffer, 1);
  sem_create(&s_item, 0);
  sem_create(&s_vaga, 5);

  task_create(&c1, consumer, "c1");
  task_create(&c2, consumer, "c2");

  task_create(&p1, producer, "p1");
  task_create(&p2, producer, "p2");
  task_create(&p3, producer, "p3");
  
  task_join(&c2);
  task_exit(0);
  
  exit(0);
}
