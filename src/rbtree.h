#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <map>
#include <set>
using namespace std;
int rand(int max) 
{
    //srand((unsigned) time(0));
    return rand()%(max);
}

#define    RB_RED        0
#define    RB_BLACK    1

//struct my_data{
//    int first,second;
//};


typedef unsigned int rel_time_t;

typedef struct _dataitem
{
    uint8_t hashkey;
    char* value;
    rel_time_t time;
    rel_time_t exptime;
    struct _dataitem *next;/*some hashvalue chain*/  
}item;

struct rb_node {
    //在rb_node中添加自己的数000
    struct uint8_t key;
    item dataitem;
    struct rb_node *rb_parent;
    int rb_color;
    struct rb_node *rb_right;
    struct rb_node *rb_left;
    //used for select and rank function. when a new rb_node is initialized, weight should be 1.
    int weight;
};

struct rb_root {
    struct rb_node *rb_node;
    int size;
};

void rb_insert_color(struct rb_node *node, struct rb_root *root);

void rb_erase(struct rb_node *node, struct rb_root *root);

/*
* This function returns the first node (in sort order) of the tree.
*/
struct rb_node *rb_first(struct rb_root *root);

struct rb_node *rb_last(struct rb_root *root);

struct rb_node *rb_next(struct rb_node *node);

struct rb_node *rb_prev(struct rb_node *node);

int getK(set<int> & si,int order);

