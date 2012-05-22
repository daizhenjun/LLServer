#include "defines.h"

typedef int elem_type;

typedef struct dnode
{
  elem_type data;
  struct dnode *prior,*next;
} dnode,*dlinklist;

status init_dlist(dlinklist *L);

status destroy_dlist(dlinklist *L);

status clear_dlist(dlinklist L);

status empty_dlist(dlinklist L);

int length_dlist(dlinklist L);

status get_delem(dlinklist L,int i,elem_type *e);

int locate_delem(dlinklist L,elem_type e,status(*compare)(elem_type,elem_type));

status locate_delem(dlinklist L,elem_type cur_e,elem_type *pre_e);

status next_delem(dlinklist L,elem_type cur_e,elem_type *next_e);

dlinklist get_delemp(dlinklist L,int i);

status insert_dlist(dlinklist L,int i,elem_type e);

status delete_dlist(dlinklist L,int i,elem_type *e);

void traverse_dlist(dlinklist L,void(*visit)(elem_type));

void traverse_back_dlist(dlinklist L,void(*visit)(elem_type));

status dlistcompare(elem_type c1,elem_type c2);

void visit_dlist(elem_type c);

