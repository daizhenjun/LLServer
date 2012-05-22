//http://apps.hi.baidu.com/share/detail/20710202
//把上一个版本改进了，现在支持size，select和rank了。
//速度上比原来稍稍慢一点，但是比stl 的set还是快20倍左右，并且stl的set不支持select和rank。
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <map>
#include <set>
#include <stdint.h>

using namespace std;
uint8_t rand(int max) /*随机函数,用于生成一个0 - max-1的整数*/
{
    //srand((unsigned) time(0));
  return (uint8_t)(rand()%(max));
}


#define    RB_RED        0
#define    RB_BLACK    1
/* 定义自己的entry，first是key，second是value。*/
//struct my_data{
//    int first,second;
//};


typedef unsigned int rel_time_t;

typedef struct _dataitem
{
    uint8_t key;
    char* value;
    rel_time_t time;
    rel_time_t exptime;
    struct _dataitem *next;/*some hashvalue chain*/  
}item;

/*定义自己的数据的比较函数*/
static int cmp(item * left, item * right){
    return left->key - right->key;
}

struct rb_node {
    //在rb_node中添加自己的数据
    item data;
    struct rb_node *rb_parent;
    int rb_color;
    struct rb_node *rb_right;
    struct rb_node *rb_left;
    //used for select and rank function. when a new rb_node is initialized, weight should be 1.
    int weight;
};

//struct rb_node {
//    //在rb_node中添加自己的数据
//    struct my_data key;
//    struct rb_node *rb_parent;
//    int rb_color;
//    struct rb_node *rb_right;
//    struct rb_node *rb_left;
//    //used for select and rank function. when a new rb_node is initialized, weight should be 1.
//    int weight;
//};

struct rb_root {
    struct rb_node *rb_node;
    int size;
};

static void __rb_rotate_left(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *right = node->rb_right;

    int a=0,b=0,c=0;
    if(node->rb_left)
        a=node->rb_left->weight;
    if(right->rb_left)
        b=right->rb_left->weight;
    if(right->rb_right)
        c=right->rb_right->weight;
    node->weight=a+b+1;
    right->weight=a+b+c+2;

    if ((node->rb_right = right->rb_left))
        right->rb_left->rb_parent = node;
    right->rb_left = node;

    if ((right->rb_parent = node->rb_parent))
    {
        if (node == node->rb_parent->rb_left)
            node->rb_parent->rb_left = right;
        else
            node->rb_parent->rb_right = right;
    }
    else
        root->rb_node = right;
    node->rb_parent = right;
}

static void __rb_rotate_right(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *left = node->rb_left;

    int a=0,b=0,c=0;
    if(node->rb_right)
        c=node->rb_right->weight;
    if(left->rb_left)
        a=left->rb_left->weight;
    if(left->rb_right)
        b=left->rb_right->weight;
    node->weight=b+c+1;
    left->weight=a+b+c+2;

    if ((node->rb_left = left->rb_right))
        left->rb_right->rb_parent = node;
    left->rb_right = node;

    if ((left->rb_parent = node->rb_parent))
    {
        if (node == node->rb_parent->rb_right)
            node->rb_parent->rb_right = left;
        else
            node->rb_parent->rb_left = left;
    }
    else
        root->rb_node = left;
    node->rb_parent = left;
}

void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *parent, *gparent;

    while ((parent = node->rb_parent) && parent->rb_color == RB_RED)
    {
        gparent = parent->rb_parent;

        if (parent == gparent->rb_left)
        {
            {
                register struct rb_node *uncle = gparent->rb_right;
                if (uncle && uncle->rb_color == RB_RED)
                {
                    uncle->rb_color = RB_BLACK;
                    parent->rb_color = RB_BLACK;
                    gparent->rb_color = RB_RED;
                    node = gparent;
                    continue;
                }
            }

            if (parent->rb_right == node)
            {
                register struct rb_node *tmp;
                __rb_rotate_left(parent, root);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            parent->rb_color = RB_BLACK;
            gparent->rb_color = RB_RED;
            __rb_rotate_right(gparent, root);
        } else {
            {
                register struct rb_node *uncle = gparent->rb_left;
                if (uncle && uncle->rb_color == RB_RED)
                {
                    uncle->rb_color = RB_BLACK;
                    parent->rb_color = RB_BLACK;
                    gparent->rb_color = RB_RED;
                    node = gparent;
                    continue;
                }
            }

            if (parent->rb_left == node)
            {
                register struct rb_node *tmp;
                __rb_rotate_right(parent, root);
                tmp = parent;
                parent = node;
                node = tmp;
            }

            parent->rb_color = RB_BLACK;
            gparent->rb_color = RB_RED;
            __rb_rotate_left(gparent, root);
        }
    }

    root->rb_node->rb_color = RB_BLACK;
}

static void __rb_erase_color(struct rb_node *node, struct rb_node *parent,
                 struct rb_root *root)
{
    struct rb_node *other;

    while ((!node || node->rb_color == RB_BLACK) && node != root->rb_node)
    {
        if (parent->rb_left == node)
        {
            other = parent->rb_right;
            if (other->rb_color == RB_RED)
            {
                other->rb_color = RB_BLACK;
                parent->rb_color = RB_RED;
                __rb_rotate_left(parent, root);
                other = parent->rb_right;
            }
            if ((!other->rb_left ||
                 other->rb_left->rb_color == RB_BLACK)
                && (!other->rb_right ||
                other->rb_right->rb_color == RB_BLACK))
            {
                other->rb_color = RB_RED;
                node = parent;
                parent = node->rb_parent;
            }
            else
            {
                if (!other->rb_right ||
                    other->rb_right->rb_color == RB_BLACK)
                {
                    register struct rb_node *o_left;
                    if ((o_left = other->rb_left))
                        o_left->rb_color = RB_BLACK;
                    other->rb_color = RB_RED;
                    __rb_rotate_right(other, root);
                    other = parent->rb_right;
                }
                other->rb_color = parent->rb_color;
                parent->rb_color = RB_BLACK;
                if (other->rb_right)
                    other->rb_right->rb_color = RB_BLACK;
                __rb_rotate_left(parent, root);
                node = root->rb_node;
                break;
            }
        }
        else
        {
            other = parent->rb_left;
            if (other->rb_color == RB_RED)
            {
                other->rb_color = RB_BLACK;
                parent->rb_color = RB_RED;
                __rb_rotate_right(parent, root);
                other = parent->rb_left;
            }
            if ((!other->rb_left ||
                 other->rb_left->rb_color == RB_BLACK)
                && (!other->rb_right ||
                other->rb_right->rb_color == RB_BLACK))
            {
                other->rb_color = RB_RED;
                node = parent;
                parent = node->rb_parent;
            }
            else
            {
                if (!other->rb_left ||
                    other->rb_left->rb_color == RB_BLACK)
                {
                    register struct rb_node *o_right;
                    if ((o_right = other->rb_right))
                        o_right->rb_color = RB_BLACK;
                    other->rb_color = RB_RED;
                    __rb_rotate_left(other, root);
                    other = parent->rb_left;
                }
                other->rb_color = parent->rb_color;
                parent->rb_color = RB_BLACK;
                if (other->rb_left)
                    other->rb_left->rb_color = RB_BLACK;
                __rb_rotate_right(parent, root);
                node = root->rb_node;
                break;
            }
        }
    }
    if (node)
        node->rb_color = RB_BLACK;
}

void rb_erase(struct rb_node *node, struct rb_root *root)
{
    root->size--;
    struct rb_node *child, *parent;
    int color;

    if (!node->rb_left)
        child = node->rb_right;
    else if (!node->rb_right)
        child = node->rb_left;
    else
    {
        struct rb_node *old = node, *left;

        node = node->rb_right;
        while ((left = node->rb_left) != NULL)
            node = left;
        child = node->rb_right;
        parent = node->rb_parent;
        color = node->rb_color;

        if (child)
            child->rb_parent = parent;
        if (parent)
        {
            if (parent->rb_left == node)
                parent->rb_left = child;
            else
                parent->rb_right = child;
        }
        else
            root->rb_node = child;

        if (node->rb_parent == old)
            parent = node;

        struct rb_node * q = parent;
        while(q){
            q->weight--;
            q=q->rb_parent;
        }

        node->rb_parent = old->rb_parent;
        node->rb_color = old->rb_color;
        node->weight = old->weight;
        node->rb_right = old->rb_right;
        node->rb_left = old->rb_left;

        if (old->rb_parent)
        {
            if (old->rb_parent->rb_left == old)
                old->rb_parent->rb_left = node;
            else
                old->rb_parent->rb_right = node;
        } else
            root->rb_node = node;

        old->rb_left->rb_parent = node;
        if (old->rb_right)
            old->rb_right->rb_parent = node;
        goto color;
    }

    parent = node->rb_parent;
    color = node->rb_color;

    if (child)
        child->rb_parent = parent;
    if (parent)
    {
        struct rb_node * q = parent;
        while(q){
            q->weight--;
            q=q->rb_parent;
        }
        if (parent->rb_left == node)
            parent->rb_left = child;
        else
            parent->rb_right = child;
    }
    else
        root->rb_node = child;

color:
    if (color == RB_BLACK)
        __rb_erase_color(child, parent, root);
}


/*
* This function returns the first node (in sort order) of the tree.
*/
struct rb_node *rb_first(struct rb_root *root)
{
    struct rb_node    *n;

    n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_left)
        n = n->rb_left;
    return n;
}

struct rb_node *rb_last(struct rb_root *root)
{
    struct rb_node    *n;

    n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_right)
        n = n->rb_right;
    return n;
}

struct rb_node *rb_next(struct rb_node *node)
{
    /* If we have a right-hand child, go down and then left as far
       as we can. */
    if (node->rb_right) {
        node = node->rb_right;
        while (node->rb_left)
            node=node->rb_left;
        return node;
    }

    /* No right-hand children. Everything down and left is
       smaller than us, so any 'next' node must be in the general
       direction of our parent. Go up the tree; any time the
       ancestor is a right-hand child of its parent, keep going
       up. First time it's a left-hand child of its parent, said
       parent is our 'next' node. */
    while (node->rb_parent && node == node->rb_parent->rb_right)
        node = node->rb_parent;

    return node->rb_parent;
}

struct rb_node *rb_prev(struct rb_node *node)
{
    /* If we have a left-hand child, go down and then right as far
       as we can. */
    if (node->rb_left) {
        node = node->rb_left;
        while (node->rb_right)
            node=node->rb_right;
        return node;
    }

    /* No left-hand children. Go up till we find an ancestor which
       is a right-hand child of its parent */
    while (node->rb_parent && node == node->rb_parent->rb_left)
        node = node->rb_parent;

    return node->rb_parent;
}

static int my_rb_insert(struct rb_node *t, struct rb_root *root)
{
    struct rb_node **p = &root->rb_node;
    struct rb_node *parent = NULL;
    int sig;

    while (*p) {
        parent = *p;
        parent->weight++;
        sig = cmp(&t->data, &parent->data);
        if (sig < 0)
            p = &(parent)->rb_left;
        else if (sig > 0)
            p = &(parent)->rb_right;
        else{
            while(parent){
                parent->weight--;
                parent=parent->rb_parent;
            }
            return 1;
        }
    }
    root->size++;
    t->rb_parent = parent;
    t->rb_color = RB_RED;
    t->rb_left = t->rb_right = NULL;
    t->weight=1;
    *p = t;
    rb_insert_color(t, root);
    return 0;
}

static struct rb_node * my_rb_find(item * key, struct rb_root *root)
{
    struct rb_node *n = root->rb_node;
    int sig;

    while (n) {
        sig = cmp(key, &n->data);
        if (sig < 0)
            n = n->rb_left;
        else if (sig > 0)
            n = n->rb_right;
        else
            return n;
    }
    return NULL;
}
/*order base is from 1 to root->size*/
static struct rb_node * select(int order, struct rb_root * root){
    if(order<=0 || order > root->size)
        return NULL;
    struct rb_node * cn=root->rb_node;
    int ls=0;
    while(cn){
        if(cn->rb_left)
            ls=cn->rb_left->weight;
        else ls=0;
        if(ls+1==order)
            return cn;
        else if(ls+1>order){
            cn=cn->rb_left;
        }else{
            order-=ls+1;
            cn=cn->rb_right;
        }
    }
    return NULL;
}
/*return a integer which is the number of rb_node->key <= w->key */
static int rank(item * w, struct rb_root * root){
    if(!root->rb_node)
        return 0;
    int res=0;
    struct rb_node * cn=root->rb_node;
    int sig;
    while(cn){
        sig = cmp(&cn->data,w);
        if(sig==0)
            return res+1+(cn->rb_left?cn->rb_left->weight:0);
        else if(sig<0){
            res+=1+(cn->rb_left?cn->rb_left->weight:0);
            cn = cn->rb_right;
        }else{
            cn = cn->rb_left;
        }
    }
    return res;
}
/*注意参数是rb_node * t,因此要删除一个元素，可能首先要my_rb_find。也可以在外部提供一个cache，
以省去my_rb_find的过程。
*/
static void my_rb_delete(struct rb_node *t, struct rb_root *root)
{
    rb_erase(t, root);
}
rb_node cache[1000000]; //for testing
int pos;
//bool test_eq(set<int> & si, rb_root * root){
//    rb_node * rbp = rb_first(root);
//    set<int>::iterator it=si.begin();
//    while(true){
//        if(rbp == NULL && it == si.end())
//            return true;
//        if(rbp != NULL && it != si.end()){
//            //printf("%d %d\n",rbp->key.first,*it);
//            if(rbp->key.first != *it)
//                return false;
//            rbp = rb_next(rbp);
//            it++;
//        }else{
//            return false;
//        }
//    }
//    return true;
//}
int getK(set<int> & si,int order){
    set<int>::iterator it=si.begin();
    for(int i=0;i<order-1;i++)
        it++;
    return *it;
}
int main(int argc, char **argv)
{
    //定义一个空的红黑树
    struct rb_root test1={0,};
    set<int> test2;
    int count=10000;
    int max_value=3000;
    pos=0;
    srand((unsigned) time(0));
    int * insert_value=new int[count];
    int * delete_value=new int[count];
    if(insert_value==NULL || delete_value ==NULL){
        printf(" not enough heap!\n");
        return 0;
    }
    for(int i=0;i<count;i++){
        insert_value[i]=rand(max_value);
        delete_value[i]=rand(max_value);
	int ii = 0;
    }
    item tmp;
    clock_t t1=clock();
    for(int i=0;i<count ;i++){
        int next=insert_value[i];
        //printf("%d next:",next);
        item *itemdata;// = (item*)malloc(sizeof(item));;
        itemdata->key = next;
     	sprintf(itemdata->value, "daizhj:%d\n", i);
        cache[pos].data = *itemdata;

        //插入
        my_rb_insert(&cache[pos],&test1);
        pos++;
    }
    for(int j=0;j<count;j++){
        int next=delete_value[j];
                
        tmp.key=next;
        //查找
        rb_node * find = my_rb_find(&tmp,&test1);
        if(find){
            //删除
            my_rb_delete(find,&test1);
        }
    }
    clock_t t2=clock();
    printf("rb time: %ld\n",t2-t1);

    clock_t t3=clock();
    for(int i=0;i<count ;i++){
        int next=insert_value[i];
        test2.insert(next);
    }
    for(int j=0;j<count;j++){
        int next=delete_value[j];
        test2.erase(next);
    }
    clock_t t4=clock();
    printf("set time: %ld\n",t4-t3);


    printf(" set size: %d rb_size: %d\n",test2.size(),test1.size);
    //if(test_eq(test2,&test1)==false){
    //    printf("error\n");
    //}
    /*int times=1000;
    while(times-->0){
        int order=rand(test1.size)+1;
        rb_node * find =select(order, & test1);
        int k=getK(test2,order);
        int rankr=rank(&find->data,&test1);
        if(rankr!=order){
            printf("error!!!!!\n");
        }
        if(find->data.key!=k){
            printf("error: %d %d\n",find->data.key,k);
            break;
        }
    }
    */
    system("pause");
    return 0;
} 
