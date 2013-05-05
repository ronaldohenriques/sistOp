// Aluno: Bruno E. O. Meneguele

#include <stdio.h>
#include "tasklist.h"

void list_append(task_t **list, task_t *task)
{
  task_t *aux, *first;
  
  first = *list;

  // Adiciona o primeiro item da lista.
  if (*list == NULL) {
    *list = task;
    (*list)->next = *list;
    (*list)->prev = *list;
    return;
  }

  // Tarefa invalida.
  if (task == NULL)  {
    //perror("Task doesn't exists.\n");
    return;
  }

  // Tarefa pertencente a outra lista.
  if (task->next != NULL || task->prev != NULL) {
    //perror("This task belongs to another list.");
    return;
  }

  do {
    aux = *list;
  } while ((*list = (*list)->next) != first);

  aux->next = task;
  task->prev = aux;
  task->next = first;
  first->prev = task;
}

task_t *list_remove(task_t **list, task_t *elem)
{
  task_t *aux;
  int count;

  aux = *list; 

  // Lista nao existente.
  if (*list == NULL) {
    //perror("List doesn't exists.\n");
    return NULL;
  }

  // Elemento invalido.
  if (elem == NULL)  {
    //perror("Invalid element.\n");
    return NULL;
  }

  // Percorre toda a lista em busca do elemento 
  // desejado.
  for (count = 0; count < list_size(*list); count++) {
    if (aux == elem) 
      break;

    aux = aux->next;

    if (count == list_size(*list) - 1) {
      //perror("Element not found.\n");
      return NULL;
    }
  }

  // Caso a lista possuir apenas um elemento faz com 
  // que o ponteiro para a lista aponte para NULL.
  if (list_size(*list) == 1) {
    *list = NULL;
    aux->next = NULL;
    aux->prev = NULL;
    return aux;
  }
  
  // Se o elemento removido for o primeiro da lista
  // faz com que o ponteiro da lista aponte para o
  // segundo (que sera o novo primeiro elemento).
  if (aux == *list)
    *list = aux->next;

  aux->prev->next = aux->next;
  aux->next->prev = aux->prev;
  aux->next = NULL;
  aux->prev = NULL;

  return aux;
}

void list_print(char *name, task_t *list)
{
  int curr_id, prev_id, next_id;
  task_t *first;

  first = list;

  printf("%s: [", name);

  do {
    if (list == NULL) 
      break;

    curr_id = list->id;
    prev_id = list->prev->id;
    next_id = list->next->id;
    printf("%d<%d>%d ", prev_id, curr_id, next_id);
  } while ((list = list->next) != first);

  printf("]\n");
}

int list_size(task_t *list)
{
  task_t *first;
  int count = 0;

  first = list;

  // Caso a lista for NULL, retorna 0.
  if (list == NULL) {
    return count;
  }

  do {
    count++;
  } while ((list = list->next) != first);

  return count;
}
