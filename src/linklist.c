#include <string.h>
#include <ctype.h>
#include <malloc.h> /* malloc()等 */
#include <limits.h> /* INT_MAX等 */
#include <stdio.h> /* EOF(=^Z或F6),NULL */
#include <stdlib.h> /* atoi() */
#include <stdio.h> /* eof() */
#include <math.h> /* floor(),ceil(),abs() */
#ifndef WIN32
#include <errno.h>
#else
#include <process.h> /* exit() */
#endif
#include "linklist.h"

status create_node(link *p,elem_type e)
{ 
	/* 分配由p指向的值为e的结点，并返回OK；若分配失败。则返回ERROR */
	*p=(link)malloc(sizeof(lnode));
	if(!*p)
	   return ERROR;
	(*p)->data=e;
	return OK;
}

void free_node(link *p)
{ /* 释放p所指结点 */
	free(*p);
	*p=NULL;
}

status init_list(linklist *L)
{ /* 构造一个空的线性链表 */
	link p;
	p=(link)malloc(sizeof(lnode)); /* 生成头结点 */
	if(p)
	{
		p->next=NULL;
		(*L).head=(*L).tail=p;
		(*L).len=0;
		return OK;
	}
	else
		return ERROR;
}

status clear_list(linklist *L)
{ /* 将线性链表L重置为空表，并释放原链表的结点空间 */
	link p,q;
	if((*L).head!=(*L).tail)/* 不是空表 */
	{
	  p=q=(*L).head->next;
	  (*L).head->next=NULL;
	  while(p!=(*L).tail)
	  {
		p=q->next;
		free(q);
		q=p;
	  }
	  free(q);
	  (*L).tail=(*L).head;
	  (*L).len=0;
	}
	return OK;
}

status destroy_list(linklist *L)
{ 
	/* 销毁线性链表L，L不再存在 */
	clear_list(L); /* 清空链表 */
	free_node(&(*L).head);
	(*L).tail=NULL;
	(*L).len=0;
	return OK;
}

status insert_first(linklist *L,link h,link s) /* 形参增加L,因为需修改L */
{ 
	/* h指向L的一个结点，把h当做头结点，将s所指结点插入在第一个结点之前 */
	s->next=h->next;
	h->next=s;
	if(h==(*L).tail) /* h指向尾结点 */
	  (*L).tail=h->next; /* 修改尾指针 */
	(*L).len++;
	return OK;
}

status delete_first(linklist *L, link h, link *q) /* 形参增加L,因为需修改L */
{ 
	/* h指向L的一个结点，把h当做头结点，删除链表中的第一个结点并以q返回。 */
	 /* 若链表为空(h指向尾结点)，q=NULL，返回FALSE */
	 *q=h->next;
	 if(*q) /* 链表非空 */
	 {
		h->next=(*q)->next;
		if(!h->next) /* 删除尾结点 */
		  (*L).tail=h; /* 修改尾指针 */
		(*L).len--;
		return OK;
	 }
	 else
	    return FALSE; /* 链表空 */
}

status append(linklist *L,link s)
{ 
	/* 将指针s(s->data为第一个数据元素)所指(彼此以指针相链,以NULL结尾)的 */
	/* 一串结点链接在线性链表L的最后一个结点之后,并改变链表L的尾指针指向新 */
	/* 的尾结点 */
	int i=1;
	(*L).tail->next=s;
	while(s->next)
	{
	  s=s->next;
	  i++;
	}
	(*L).tail=s;
	(*L).len+=i;
	return OK;
}

pos prior_pos(linklist L,link p)
{ 
	/* 已知p指向线性链表L中的一个结点，返回p所指结点的直接前驱的位置 */
	/* 若无前驱，则返回NULL */
	link q;
	q=L.head->next;
	if(q==p) /* 无前驱 */
		return NULL;
	else
	{
		while(q->next!=p) /* q不是p的直接前驱 */
		  q=q->next;
		return q;
	}
}

status remove(linklist *L,link *q)
{ 
	/* 删除线性链表L中的尾结点并以q返回，改变链表L的尾指针指向新的尾结点 */
	link p=(*L).head;
	if((*L).len==0) /* 空表 */
	{
		*q=NULL;
		return FALSE;
	}
	while(p->next!=(*L).tail)
		p=p->next;
	*q=(*L).tail;
	p->next=NULL;
	(*L).tail=p;
	(*L).len--;
	return OK;
}

status insert_before(linklist *L,link *p,link s)
{
	/* 已知p指向线性链表L中的一个结点，将s所指结点插入在p所指结点之前， */
	/* 并修改指针p指向新插入的结点 */
	link q;
	q=prior_pos(*L,*p); /* q是p的前驱 */
	if(!q) /* p无前驱 */
		q=(*L).head;
	s->next=*p;
	q->next=s;
	*p=s;
	(*L).len++;
	return OK;
}

status insert_after(linklist *L,link *p,link s)
{ 
	/* 已知p指向线性链表L中的一个结点，将s所指结点插入在p所指结点之后， */
	/* 并修改指针p指向新插入的结点 */
	if(*p==(*L).tail) /* 修改尾指针 */
		(*L).tail=s;
	s->next=(*p)->next;
	(*p)->next=s;
	*p=s;
	(*L).len++;
	return OK;
}

status set_curelem(link p,elem_type e)
{
	/* 已知p指向线性链表中的一个结点，用e更新p所指结点中数据元素的值 */
    p->data=e;
    return OK;
}

elem_type get_curelem(link p)
{ 
	/* 已知p指向线性链表中的一个结点，返回p所指结点中数据元素的值 */
    return p->data;
}

status empty_list(linklist L)
{ 
	/* 若线性链表L为空表，则返回TRUE，否则返回FALSE */
	if(L.len)
		return FALSE;
	else
		return TRUE;
}

int list_length(linklist L)
{ 
	/* 返回线性链表L中元素个数 */
    return L.len;
}

pos list_head(linklist L)
{ 
	/* 返回线性链表L中头结点的位置 */
    return L.head;
}

pos get_last(linklist L)
{
	/* 返回线性链表L中最后一个结点的位置 */
    return L.tail;
}

pos next_pos(link p)
{ 
	/* 已知p指向线性链表L中的一个结点，返回p所指结点的直接后继的位置 */
    /* 若无后继，则返回NULL */
    return p->next;
}

status locate_pos(linklist L,int i,link *p)
{
	/* 返回p指示线性链表L中第i个结点的位置，并返回OK，i值不合法时返回ERROR */
	/* i=0为头结点 */
	int j;
	if(i<0||i>L.len)
		return ERROR;
	else
	{
		*p=L.head;
		for(j=1;j<=i;j++)
		  *p=(*p)->next;
		return OK;
	}
}

pos locate_elem(linklist L,elem_type e,status (*compare)(elem_type,elem_type))
{ 
	/* 返回线性链表L中第1个与e满足函数compare()判定关系的元素的位置， */
	/* 若不存在这样的元素，则返回NULL */
	link p=L.head;
	do
		p=p->next;
	while(p&&!(compare(p->data,e))); /* 没到表尾且没找到满足关系的元素 */
	return p;
}

status list_traverse(linklist L,void(*visit)(elem_type))
{ 
	/* 依次对L的每个数据元素调用函数visit()。一旦visit()失败，则操作失败 */
	link p=L.head->next;
	int j;
	for(j=1;j<=L.len;j++)
	{
		visit(p->data);
		p=p->next;
	}
	printf("\n");
	return OK;
}

status order_insert(linklist *L,elem_type e,int (*comp)(elem_type,elem_type))
{ 
	/* 已知L为有序线性链表，将元素e按非降序插入在L中。（用于一元多项式） */
	link o,p,q;
	q=(*L).head;
	p=q->next;
	while(p!=NULL&&comp(p->data,e)<0) /* p不是表尾且元素值小于e */
	{
		q=p;
		p=p->next;
	}
	o=(link)malloc(sizeof(lnode)); /* 生成结点 */
	o->data=e; /* 赋值 */
	q->next=o; /* 插入 */
	o->next=p;
	(*L).len++; /* 表长加1 */
	if(!p) /* 插在表尾 */
		(*L).tail=o; /* 修改尾结点 */
	return OK;
}

status locate_elemp(linklist L,elem_type e,pos *q,int(*compare)(elem_type,elem_type))
{ 
	/* 若升序链表L中存在与e满足判定函数compare()取值为0的元素，则q指示L中 */
	/* 第一个值为e的结点的位置，并返回TRUE；否则q指示第一个与e满足判定函数 */
	/* compare()取值>0的元素的前驱的位置。并返回FALSE。（用于一元多项式） */
	link p=L.head,pp;
	do
	{
		pp=p;
		p=p->next;
	}while(p&&(compare(p->data,e)<0)); /* 没到表尾且p->data.expn<e.expn */

	if(!p||compare(p->data,e)>0) /* 到表尾或compare(p->data,e)>0 */
	{
		*q=pp;
		return FALSE;
	}
	else /* 找到 */
	{
		*q=p;
		return TRUE;
	}
}



status compare(elem_type c1,elem_type c2) /* c1等于c2 */
{
	if(c1==c2)
		return TRUE;
	else
		return FALSE;
}

int cmp(elem_type a,elem_type b)
{
	  /* 根据a<、=或>b,分别返回-1、0或1 */
	if(a==b)
		return 0;
	else
		return (a-b)/abs(a-b);
}

void visit(elem_type c)
{
	printf("%d ",c);
}

//int _tmain(int argc, _TCHAR* argv[])
//{
//   link p,h;
//   linklist L;
//   status i;
//   int j,k;
//   i=init_list(&L);
//   if(!i) /* 初始化空的线性表L不成功 */
//     exit(FALSE); /* 退出程序运行 */
//   for(j=1;j<=2;j++)
//   {
//     create_node(&p,j); /* 生成由p指向、值为j的结点 */
//     insert_first(&L,L.tail,p); /* 插在表尾 */
//   }
//   order_insert(&L,0,cmp); /* 按升序插在有序表头 */
//   for(j=0;j<=3;j++)
//   {
//     i=locate_elemp(L,j,&p,cmp);
//     if(i)
//       printf("链表中有值为%d的元素。\n",p->data);
//     else
//       printf("链表中没有值为%d的元素。\n",j);
//   }
//   printf("输出链表：");
//   list_traverse(L,visit); /* 输出L */
//   for(j=1;j<=4;j++)
//   {
//     printf("删除表头结点：");
//     delete_first(&L,L.head,&p); /* 删除L的首结点，并以p返回 */
//     if(p)
//       printf("%d\n",get_curelem(p));
//     else
//       printf("表空，无法删除 p=%u\n",p);
//   }
//   printf("L中结点个数=%d L是否空 %d(1:空 0:否)\n",list_length(L),empty_list(L));
//   create_node(&p,10);
//   p->next=NULL; /* 尾结点 */
//   for(j=4;j>=1;j--)
//   {
//     create_node(&h,j*2);
//     h->next=p;
//     p=h;
//   } /* h指向一串5个结点，其值依次是2 4 6 8 10 */
//   append(&L,h); /* 把结点h链接在线性链表L的最后一个结点之后 */
//   order_insert(&L,12,cmp); /* 按升序插在有序表尾头 */
//   order_insert(&L,7,cmp); /* 按升序插在有序表中间 */
//   printf("输出链表：");
//   list_traverse(L,visit); /* 输出L */
//   for(j=1;j<=2;j++)
//   {
//     p=locate_elem(L,j*5,compare);
//     if(p)
//       printf("L中存在值为%d的结点。\n",j*5);
//     else
//       printf("L中不存在值为%d的结点。\n",j*5);
//   }
//   for(j=1;j<=2;j++)
//   {
//     locate_pos(L,j,&p); /* p指向L的第j个结点 */
//     h=prior_pos(L,p); /* h指向p的前驱 */
//     if(h)
//       printf("%d的前驱是%d。\n",p->data,h->data);
//     else
//       printf("%d没前驱。\n",p->data);
//   }
//   k=list_length(L);
//   for(j=k-1;j<=k;j++)
//   {
//     locate_pos(L,j,&p); /* p指向L的第j个结点 */
//     h=next_pos(p); /* h指向p的后继 */
//     if(h)
//       printf("%d的后继是%d。\n",p->data,h->data);
//     else
//       printf("%d没后继。\n",p->data);
//   }
//   printf("L中结点个数=%d L是否空 %d(1:空 0:否)\n",list_length(L),empty_list(L));
//   p=get_last(L); /* p指向最后一个结点 */
//   set_curelem(p,15); /* 将最后一个结点的值变为15 */
//   printf("第1个元素为%d 最后1个元素为%d\n",get_curelem(list_head(L)->next),get_curelem(p));
//   create_node(&h,10);
//   insert_before(&L,&p,h); /* 将10插到尾结点之前，p指向新结点 */
//   p=p->next; /* p恢复为尾结点 */
//   create_node(&h,20);
//   insert_after(&L,&p,h); /* 将20插到尾结点之后 */
//   k=list_length(L);
//   printf("依次删除表尾结点并输出其值：");
//   for(j=0;j<=k;j++)
//   {
//     i=remove(&L,&p);
//     if(!i) /* 删除不成功 */
//       printf("删除不成功 p=%u\n",p);
//     else
//       printf("%d ",p->data);
//   }
//   create_node(&p,29); /* 重建具有1个结点(29)的链表 */
//   insert_first(&L,L.head,p);
//   destroy_list(&L); /* 销毁线性链表L */
//   printf("销毁线性链表L之后: L.head=%u L.tail=%u L.len=%d\n",L.head,L.tail,L.len);
//}
