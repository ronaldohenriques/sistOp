#include <stdio.h>
#include "tasklist.h"

void list_append(task_t **list, task_t *task)
{
  task_t *aux, *first;

  first = *list;
  //printf("Entrou no list_append()\n");

  if (*list == NULL) {
    printf("First task of the list.\n");
    *list = task;
    (*list)->next = *list;
    (*list)->prev = *list;
    first = *list;
    return;
  }

  if (task == NULL)  {
    perror("Task doesn't exists.\n");
    return;
  }

  do {
    aux = *list;
  } while ((*list = (*list)->next) != first);
  
  aux->next = task;
  task->prev = aux;
  task->next = first;
  first->prev = task;
  //printf("Saiu do list_append()\n");
}

task_t *list_remove(task_t **list, task_t *elem)
{
  task_t *aux, *aux_prev, *aux_next;
  int count;

  if (*list == NULL) {
    perror("List doesn't exists.\n");
    return NULL;
  }

 if (elem == NULL)  {
    perror("Invalid element.\n");
    return NULL;
  }

  printf("list_size(*list) = %d\n", list_size(*list));
  
  for (count = 0; count < list_size(*list); count++) {
    aux = *list;

    printf("count = %d\n", count);

    if (aux == elem) {
      printf("if(aux == elem) -> TRUE\n");
      break;
    }

    *list = (*list)->next;

    if (count == list_size(*list) - 1) {
      perror("Element not found.\n");
      return NULL;
    }
  }

  if (list_size(*list) == 1) {
    *list = NULL;
    aux->next = NULL;
    aux->prev = NULL;
    return aux;
  }

  aux_prev = aux->prev;
  aux_next = aux->next;
  aux_prev->next = aux_next;
  aux_next->prev = aux_prev;
  aux->next = NULL;
  aux->prev = NULL;
  *list = aux_next;

  return aux;
}

void list_print(char *name, task_t *list)
{
  int curr_id, prev_id, next_id;

  printf("%s: [", name);

  do {
    if (list == NULL) 
      break;

    curr_id = list->id;
    prev_id = list->prev->id;
    next_id = list->next->id;
    printf("%d<%d>%d ", prev_id, curr_id, next_id);
  } while ((list = list->next) != NULL);

  printf("]\n");
}

int list_size(task_t *list)
{
  task_t *first;
  int count = 0;

  first = list;

  if (list == NULL) {
    //printf("count: %d\n", count);
    return count;
  }

  do {
    count++;
    //printf("count: %d\n", count);
  } while ((list = list->next) != first);

  return count;
}
