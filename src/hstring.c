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
#include "hstring.h"

hstring hs_init(char *data)
{
	hstring str;
	/* 初始化(产生空串)字符串T。另加 */
	if(data != NULL)
	{
	  str.length = strlen(data);
	  str.ch = data;
	}
	else
    {
	  str.length = 0;
	  str.ch = NULL;
	}
	return str;
}


void hs_init(hstring *pstr)
{ /* 初始化(产生空串)字符串T。另加 */
	(*pstr).length = 0;
	(*pstr).ch = NULL;
}


status hs_assign(hstring *pstr, char *chars)
{ /* 生成一个其值等于串常量chars的串T */
	int i = strlen(chars); /* 求chars的长度i */
	if((*pstr).ch)
		free((*pstr).ch); /* 释放T原有空间 */

	if(!i) { /* chars的长度为0 */
		(*pstr).ch = NULL;
		(*pstr).length = 0;
	}
	else { /* chars的长度不为0 */
		(*pstr).ch = (char*)malloc(i*sizeof(char)); /* 分配串空间 */
		if(!(*pstr).ch) /* 分配串空间失败 */
			exit(OVERFLOW);
		for(int j = 0 ;j < i ;j++) /* 拷贝串 */
			(*pstr).ch[j] = chars[j];
		(*pstr).length = i;
	}
	return OK;
}

status hs_copy(hstring *pstr,hstring s)
{ /* 初始条件:串S存在。操作结果: 由串S复制得串T */
	if((*pstr).ch)
		free((*pstr).ch); /* 释放T原有空间 */
	(*pstr).ch=(char*)malloc(s.length*sizeof(char)); /* 分配串空间 */
	if(!(*pstr).ch) /* 分配串空间失败 */
		exit(OVERFLOW);
	for(int i = 0 ;i < s.length ;i++) /* 拷贝串 */
		(*pstr).ch[i] = s.ch[i];
	(*pstr).length = s.length;
	return OK;
}

status hs_empty(hstring s)
{ /* 初始条件: 串s存在。操作结果: 若s为空串,则返回TRUE,否则返回FALSE */
	if(s.length==0&&s.ch==NULL)
		return TRUE;
	else
		return FALSE;
}

int hs_compare(hstring s1,hstring s2)
{ /* 若s1>s2,则返回值>0;若s1=s2,则返回值=0;若s1<s2,则返回值<0 */
	int i;
	for(i=0;i<s1.length&&i<s2.length;++i)
		if(s1.ch[i]!=s2.ch[i])
			return s1.ch[i]-s2.ch[i];
	return s1.length-s2.length;
}

int hs_length(hstring s)
{ /* 返回S的元素个数,称为串的长度 */
	return s.length;
}

status hs_clear(hstring *pstr)
{ /* 将S清为空串*/
	if((*pstr).ch)
	{
		free((*pstr).ch);
		(*pstr).ch=NULL;
	}
	(*pstr).length=0;
	return OK;
}

status hs_concat(hstring *pstr,hstring s1,hstring s2)
{ /* 用T返回由s1和s2联接而成的新串 */
	int i;
	if((*pstr).ch)
		free((*pstr).ch); /* 释放旧空间 */
	(*pstr).length=s1.length+s2.length;
	(*pstr).ch=(char *)malloc((*pstr).length*sizeof(char));
	if(!(*pstr).ch)
		exit(OVERFLOW);
	for(i=0;i<s1.length;i++)
		(*pstr).ch[i]=s1.ch[i];
	for(i=0;i<s2.length;i++)
		(*pstr).ch[s1.length+i]=s2.ch[i];
	return OK;
}

status hs_substring(hstring *pstr, hstring s,int pos,int len)
{ /* 用sub返回串s的第pos个字符起长度为len的子串。*/
	/* 其中,1<=pos<=hs_length(s)且<=len<=hs_strlength(s)-pos+1 */
	if(pos<1||pos>s.length||len<0||len>s.length-pos+1)
		return ERROR;
	if((*pstr).ch)
		free((*pstr).ch); /* 释放旧空间*/
	if(!len) /* 空子串*/
	{
		(*pstr).ch = NULL;
		(*pstr).length = 0;
	}
	else
	{ /* 完整子串*/
		(*pstr).ch = (char*)malloc((len+1)*sizeof(char));
		if(!(*pstr).ch)
			exit(OVERFLOW);
		for(int i = 0; i <= len-1; i++)
			(*pstr).ch[i] = s.ch[pos-1+i];
		(*pstr).length=len;
		(*pstr).ch[len]='\0';
	}
	return OK;
}


int hs_find(hstring s1,hstring s2,int pos) 
{   /* s2为非空串。若主串s1中第pos个字符之后存在与s2相等的子串, */
	/* 则返回第一个这样的子串在s1中的位置,否则返回0 */
	int n,m,i;
	hstring sub;
	hs_init(&sub);
	if(pos>0){
		n=hs_length(s1);
		m=hs_length(s2);
		i=pos;
		while(i <= (n-m+1)){
			hs_substring(&sub, s1, i, m);
			if(hs_compare(sub, s2)!=0)
				++i;
			else
				return i;
		}
	}
	return 0;
}

int hs_findfirst(hstring s1, hstring s2)
{
   return hs_find(s1, s2, 1);
}


int hs_findlast(hstring s1, hstring s2)
{
   if(s1.length == 0 || s1.length < s2.length)
	   return FALSE;

   int lastfindpos = hs_find(s1, s2, 1);
   if(lastfindpos == 0)
	   return 0;
   //还有多少字符可供查找
   int toend = s1.length - lastfindpos - s2.length;
   int findpos = 0;
   
   while(toend >= s2.length){
	   findpos = hs_find(s1, s2, lastfindpos + s2.length);
	   if(findpos)
	   {
          lastfindpos = findpos;
		  toend = s1.length - findpos - s2.length;
	   }
	   else
	      break; 
   }
   return lastfindpos;
}

int hs_findcount(hstring s1, hstring s2)
{
   if(s1.length == 0 || s1.length < s2.length)
	   return FALSE;

   int lastfindpos = hs_find(s1, s2, 1);
   if(lastfindpos == 0)
	   return 0;
   //还有多少字符可供查找
   int toend = s1.length - lastfindpos - s2.length;
   int findpos = 0;
   int count = 1;//之前lastfindpos!=0表示已查到一个 
   while(toend >= s2.length){
	   findpos = hs_find(s1, s2, lastfindpos + s2.length);
	   if(findpos)
	   {
          lastfindpos = findpos;
		  toend = s1.length - findpos - s2.length;
		  count++;
	   }
	   else
	      break; 
   }
   return count;
}

status hs_insert(hstring *pstr, int pos, hstring s) 
{ /* 1≤pos≤hs_length(pstr)+1。在串pstr的第pos个字符之前插入串s */
	int i;
	if(pos<1||pos>(*pstr).length+1) /* pos不合法 */
		return ERROR;
	if(s.length) {/* s非空,则重新分配空间,插入s */
		(*pstr).ch=(char*)realloc((*pstr).ch,((*pstr).length+s.length)*sizeof(char));
		if(!(*pstr).ch)
			exit(OVERFLOW);
		for(i = (*pstr).length-1; i>=pos-1; --i) /* 为插入T而腾出位置 */
			(*pstr).ch[i+s.length] = (*pstr).ch[i];
		for(i = 0; i<s.length; i++)
			(*pstr).ch[pos-1+i] = s.ch[i]; /* 插入s */
		(*pstr).length += s.length;
	}
	return OK;
}

status hs_delete(hstring *pstr,int pos,int len)
{ /* 从串S中删除第pos个字符起长度为len的子串 */
	if((*pstr).length<pos+len-1)
		exit(ERROR);
	for(int i = pos-1; i<=(*pstr).length-len; i++)
		(*pstr).ch[i] = (*pstr).ch[i+len];
	(*pstr).length -= len;
	(*pstr).ch = (char*)realloc((*pstr).ch, (*pstr).length * sizeof(char));
	return OK;
}

status hs_replace(hstring *pstr,hstring s1,hstring s2)
{   /* 初始条件: 串pstr, s1和s2存在,s1是非空串（此函数与串的存储结构无关） */
	/* 操作结果: 用s1替换主串pstr中出现的所有与s1相等的不重叠的子串 */
	int i=1; /* 从串S的第一个字符起查找串s1 */
	if(hs_empty(s1)) /* s1是空串 */
		return ERROR;
	do
	{
		i=hs_find(*pstr,s1,i); /* 结果i为从上一个i之后找到的子串T的位置 */
		if(i) /* 串pstr中存在串s1 */
		{
			hs_delete(pstr, i, hs_length(s1)); /* 删除该串s1 */
			hs_insert(pstr, i, s2); /* 在原串s1的位置插入串s2 */
			i += hs_length(s2); /* 在插入的串s2后面继续查找串s1 */
		}
	} while(i);
	return OK;
}


void hs_print(hstring s)
{ /* 输出s字符串。另加 */
	for(int i = 0 ;i < s.length ;i++)
		printf("%c",s.ch[i]);
	printf("\n");
}


status hs_startwith(hstring s1,hstring s2)
{   /* s2为非空串。若主串s1中第pos个字符之后存在与s2相等的子串, */	
	if(s1.length == 0 || s1.length < s2.length)
	   return FALSE;
    int i;
	for(i = 0; i < s2.length ; i++){
		if(s1.ch[i] != s2.ch[i])
		   return FALSE;
	}
	return TRUE;
}

status hs_endwith(hstring s1,hstring s2)
{   /* s2为非空串。若主串s1中第pos个字符之后存在与s2相等的子串, */	
	if(s1.length == 0 || s1.length < s2.length)
	   return FALSE;
    int i;
	for(i = 1; i <= s2.length ; i++){
		if(s1.ch[s1.length- i] != s2.ch[s2.length - i])
		   return FALSE;
	}
	return TRUE;
}


/*
int main(int argc, char *argv[])
{
	int i;
	char c,*p="God bye!",*q="God luck!";
	hstring t,s,r;
	hs_init(&t); //hstring类型必需初始化
	hs_init(&s);
	hs_init(&r);
	hs_assign(&t,p);
	printf("串t为: ");
	hs_print(t);
	printf("串长为%d 串空否？%d(1:空 0:否)\n",hs_length(t),hs_empty(t));
	hs_assign(&s,q);
	printf("串s为: ");
	hs_print(s);
	i=hs_compare(s,t);
	if(i<0)
	  c='<';
	else if(i==0)
	  c='=';
	else
	  c='>';
	printf("串s%c串t\n",c);
	hs_concat(&r,t,s);
	printf("串t联接串s产生的串r为: ");
	hs_print(r);
	hs_assign(&s,"oo");
	printf("串s为: ");
	hs_print(s);
	hs_assign(&t,"o");
	printf("串t为: ");
	hs_print(t);
	hs_replace(&r,t,s);
	printf("把串r中和串t相同的子串用串s代替后，串r为:\n");
	hs_print(r);
	hs_clear(&s);
	printf("串s清空后，串长为%d 空否？%d(1:空 0:否)\n",hs_length(s),hs_empty(s));
	hs_substring(&s,r,6,4);
	printf("串s为从串r的第6个字符起的4个字符，长度为%d 串s为: ",s.length);
	hs_print(s);
	hs_copy(&t,r);
	printf("复制串t为串r,串t为: ");
	hs_print(t);
	hs_insert(&t,6,s);
	printf("在串t的第6个字符前插入串s后，串t为: ");
	hs_print(t);
	hs_delete(&t, 1, 5);
	printf("从串t的第1个字符起删除5个字符后,串t为: ");
	hs_print(t);
	printf("%d是从串t的第1个字符起，和串s相同的第1个子串的位置\n",hs_find(t,s,1));
	printf("%d是从串t的第2个字符起，和串s相同的第1个子串的位置\n",hs_find(t,s,2));

	hs_assign(&t,"hello world!");
    hs_assign(&s,"hello");  
	printf("t是否以s开头:%d\n",hs_startwith(t, s));

	hs_assign(&s,"world!");  
	printf("t是否以s结束:%d\n",hs_endwith(t, s));

    hs_assign(&t,"wwww www ww www wwwwwww");
    hs_assign(&s,"ww");  
	printf("t中最后一次出现s的位置:%d\n",hs_findlast(t, s));
	printf("t中出现s的次数:%d\n",hs_findcount(t, s));

    return 0;
}
*/
