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
#include "dlinklist.h"

status init_dlist(dlinklist *L)
{ 
	/* 产生空的双向循环链表L */
	*L=(dlinklist)malloc(sizeof(dnode));
	if(*L)
	{
	   (*L)->next=(*L)->prior=*L;
	   return OK;
	}
	else
	  return OVERFLOW;
}

status destroy_dlist(dlinklist *L)
{
	/* 操作结果：销毁双向循环链表L */
    dlinklist q,p=(*L)->next; /* p指向第一个结点 */
	while(p!=*L) /* p没到表头 */
	{
		q=p->next;
		free(p);
		p=q;
	}
	free(*L);
	*L=NULL;
	return OK;
}

status clear_dlist(dlinklist L) /* 不改变L */
{
	/* 初始条件：L已存在。操作结果：将L重置为空表 */
	dlinklist q,p=L->next; /* p指向第一个结点 */
	while(p!=L) /* p没到表头 */
	{
		q=p->next;
		free(p);
		p=q;
	}
	L->next=L->prior=L; /* 头结点的两个指针域均指向自身 */
	return OK;
}

status empty_dlist(dlinklist L)
{
	/* 初始条件：线性表L已存在。操作结果：若L为空表，则返回TRUE，否则返回FALSE */
	if(L->next==L&&L->prior==L)
	  return TRUE;
	else
	  return FALSE;
}

int length_dlist(dlinklist L)
{
	/* 初始条件：L已存在。操作结果：返回L中数据元素个数 */
	int i=0;
	dlinklist p=L->next; /* p指向第一个结点 */
	while(p!=L) /* p没到表头 */
	{
		i++;
		p=p->next;
	}
	return i;
}

status get_delem(dlinklist L,int i,elem_type *e)
{
	/* 当第i个元素存在时,其值赋给e并返回OK,否则返回ERROR */
	int j=1; /* j为计数器 */
	dlinklist p=L->next; /* p指向第一个结点 */
	while(p!=L&&j<i) /* 顺指针向后查找,直到p指向第i个元素或p指向头结点 */
	{
		p=p->next;
		j++;
	}
	if(p==L||j>i) /* 第i个元素不存在 */
		return ERROR;
	*e=p->data; /* 取第i个元素 */
	return OK;
}

int locate_delem(dlinklist L,elem_type e,status(*compare)(elem_type,elem_type))
{
	/* 初始条件：L已存在，compare()是数据元素判定函数 */
	/* 操作结果：返回L中第1个与e满足关系compare()的数据元素的位序。 */
	/*           若这样的数据元素不存在，则返回值为0 */
	int i=0;
	dlinklist p=L->next; /* p指向第1个元素 */
	while(p!=L)
	{
		i++;
		if(compare(p->data,e)) /* 找到这样的数据元素 */
			return i;
		p=p->next;
	}
	return 0;
}

status locate_delem(dlinklist L,elem_type cur_e,elem_type *pre_e)
{ 
	/* 操作结果：若cur_e是L的数据元素，且不是第一个，则用pre_e返回它的前驱， */
	/*           否则操作失败，pre_e无定义 */
	dlinklist p=L->next->next; /* p指向第2个元素 */
	while(p!=L) /* p没到表头 */
	{
	  if(p->data==cur_e)
	  {
		*pre_e=p->prior->data;
		return TRUE;
	  }
	  p=p->next;
	}
	return FALSE;
}

status next_delem(dlinklist L,elem_type cur_e,elem_type *next_e)
{ 
	/* 操作结果：若cur_e是L的数据元素，且不是最后一个，则用next_e返回它的后继， */
	/*           否则操作失败，next_e无定义 */
	dlinklist p=L->next->next; /* p指向第2个元素 */
	while(p!=L) /* p没到表头 */
	{
	  if(p->prior->data==cur_e)
	  {
		*next_e=p->data;
		return TRUE;
	  }
	  p=p->next;
	}
	return FALSE;
}

dlinklist get_delemp(dlinklist L,int i) /* 另加 */
{
	/* 在双向链表L中返回第i个元素的位置指针(算法2.18、2.19要调用的函数) */
	int j;
	dlinklist p=L;
	for(j=1;j<=i;j++)
	  p=p->next;
	return p;
}

status insert_dlist(dlinklist L,int i,elem_type e) /* 改进算法2.18 */
{ 
	/* 在带头结点的双链循环线性表L中第i个位置之前插入元素e，i的合法值为1≤i≤表长+1 */
	dlinklist p,s;
	if(i<1||i>length_dlist(L)+1) /* i值不合法 */
	  return ERROR;
	p=get_delemp(L,i-1); /* 在L中确定第i-1个元素的位置指针p */
	if(!p) /* p=NULL,即第i-1个元素不存在 */
	  return ERROR;
	s=(dlinklist)malloc(sizeof(dnode));
	if(!s)
	  return OVERFLOW;
	s->data=e; /* 在第i-1个元素之后插入 */
	s->prior=p;
	s->next=p->next;
	p->next->prior=s;
	p->next=s;
	return OK;
}

status delete_dlist(dlinklist L,int i,elem_type *e) /* 算法2.19 */
{
	/* 删除带头结点的双链循环线性表L的第i个元素,i的合法值为1≤i≤表长+1 */
	dlinklist p;
	if(i<1||i>length_dlist(L)) /* i值不合法 */
	  return ERROR;
	p=get_delemp(L,i);  /* 在L中确定第i个元素的位置指针p */
	if(!p) /* p=NULL,即第i个元素不存在 */
	  return ERROR;
	*e=p->data;
	p->prior->next=p->next;
	p->next->prior=p->prior;
	free(p);
	return OK;
}

void traverse_dlist(dlinklist L,void(*visit)(elem_type))
{ 
	/* 由双链循环线性表L的头结点出发,正序对每个数据元素调用函数visit() */
	dlinklist p=L->next; /* p指向头结点 */
	while(p!=L)
	{
	  visit(p->data);
	  p=p->next;
	}
	printf("\n");
}

void traverse_back_dlist(dlinklist L,void(*visit)(elem_type))
{
	/* 由双链循环线性表L的头结点出发,逆序对每个数据元素调用函数visit()。另加 */
	dlinklist p=L->prior; /* p指向尾结点 */
	while(p!=L)
	{
	  visit(p->data);
	  p=p->prior;
	}
	printf("\n");
}


status dlistcompare(elem_type c1,elem_type c2) /* 数据元素判定函数(判定相等) */
{
	if(c1==c2)
	  return TRUE;
	else
	  return FALSE;
}

void visit_dlist(elem_type c) /* traverse_dlist()调用的函数(类型一致) */
{
	printf("%d ",c);
}

//int _tmain(int argc, _TCHAR* argv[])
//{
//  dlinklist L;
//   int i,n;
//   status j;
//   elem_type e;
//   init_dlist(&L);
//   for(i=1;i<=5;i++)
//     insert_dlist(L,i,i); /* 在第i个结点之前插入i */
//   printf("正序输出链表：");
//   traverse_dlist(L,visit_dlist); /* 正序输出 */
//   printf("逆序输出链表：");
//   traverse_back_dlist(L,visit_dlist); /* 逆序输出 */
//   n=2;
//   delete_dlist(L,n,&e); /* 删除并释放第n个结点 */
//   printf("删除第%d个结点，值为%d，其余结点为：",n,e);
//   traverse_dlist(L,visit_dlist); /* 正序输出 */
//   printf("链表的元素个数为%d\n",length_dlist(L));
//   printf("链表是否空：%d(1:是 0:否)\n",empty_dlist(L));
//   clear_dlist(L); /* 清空链表 */
//   printf("清空后，链表是否空：%d(1:是 0:否)\n",empty_dlist(L));
//   for(i=1;i<=5;i++)
//     insert_dlist(L,i,i); /* 重新插入5个结点 */
//   traverse_dlist(L,visit_dlist); /* 正序输出 */
//   n=3;
//   j=get_delem(L,n,&e); /* 将链表的第n个元素赋值给e */
//   if(j)
//     printf("链表的第%d个元素值为%d\n",n,e);
//   else
//     printf("不存在第%d个元素\n",n);
//   n=4;
//   i=locate_delem(L,n,dlistcompare);
//   if(i)
//     printf("等于%d的元素是第%d个\n",n,i);
//   else
//     printf("没有等于%d的元素\n",n);
//   j=locate_delem(L,n,&e);
//   if(j)
//     printf("%d的前驱是%d\n",n,e);
//   else
//     printf("不存在%d的前驱\n",n);
//   j=next_delem(L,n,&e);
//   if(j)
//     printf("%d的后继是%d\n",n,e);
//   else
//     printf("不存在%d的后继\n",n);
//   destroy_dlist(&L);
//}
