#ifndef _RBTREE_H_INCLUDED_
#define _RBTREE_H_INCLUDED_

typedef long long msec64;
typedef struct rbtreeNode_st  *rbtreeNode_t;
typedef struct rbtree_st  *rbtree_t;

struct rbtreeNode_st {
    msec64        key;
    rbtreeNode_t     left;
    rbtreeNode_t     right;
    rbtreeNode_t     parent;
    int                 color;
    int                 data;
};

struct rbtree_st {
    rbtreeNode_t     root;
    rbtreeNode_t     sentinel;
};


#define rbtree_init(tree, s)                                           \
    rbtree_stentinel_init(s);                                              \
    (tree)->root = s;                                                         \
    (tree)->sentinel = s                                                    

void rbtree_insert(rbtree_t tree, rbtreeNode_t node);
void rbtree_delete(rbtree_t tree, rbtreeNode_t node);
rbtreeNode_t rbtree_min(rbtreeNode_t node, rbtreeNode_t sentinel);

rbtreeNode_t rbtree_next(rbtree_t tree,
    rbtreeNode_t node);


#define rbt_red(node)               ((node)->color = 1)
#define rbt_black(node)             ((node)->color = 0)
#define rbt_is_red(node)            ((node)->color)
#define rbt_is_black(node)          (!rbt_is_red(node))
#define rbt_copy_color(n1, n2)      (n1->color = n2->color)


/* a sentinel must be black */

#define rbtree_stentinel_init(node)  rbt_black(node)


#endif /* _RBTREE_H_INCLUDED_ */
