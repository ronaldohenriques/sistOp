// Versão 1, 2008
// Prof. Carlos A. Maziero

#ifndef __TASKLIST__
#define __TASKLIST__

#ifndef NULL
#define NULL ((void *) 0)
#endif

#include "task.h"

// Insere um elemento (tarefa) no final da lista.
// Condicoes a verificar, gerando msgs de erro:
// - a lista deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra lista

void list_append (task_t **list, task_t *task) ;

// Remove o elemento indicado da lista, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a lista deve existir
// - a lista nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a lista indicada
// Retorno: apontador para o elemento removido, ou NULL se erro

task_t *list_remove (task_t **list, task_t *elem) ;

// Imprime o conteudo (id) da lista, precedida de seu nome.
// Esta funcao deve gerar uma listagem que apresente
// a estrutura da lista, como mostra este exemplo:
//
// Prontos: [5<0>1 0<1>2 1<2>3 2<3>4 3<4>5 4<5>0]
//
// Observe que cada item representa um elemento da lista,
// com seu precedente e seu sucessor: pre<elem>suc

void list_print (char *name, task_t *list) ;

// Conta o numero de elementos na lista
// Retorno: numero de elementos na lista

int list_size (task_t *list) ;

#endif
