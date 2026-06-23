/*
 *  quadtree.h
 */

#ifndef _QUADTREE_H
#define _QUADTREE_H 1

#include "bounding_box.h"
#include "entity.h"

#include <cstddef>

static ptrdiff_t g_node_count = 0;

#define QT_NODE_MAX 2
struct QuadTree {
    AABB bounds;
    QuadTree *child[4];
    Entity *entities[QT_NODE_MAX];
    ptrdiff_t n_entities;
    Vec2 center_of_mass;
    double total_mass;
    bool is_leaf;
    bool is_tree_root;
};

QuadTree* QT_Create(Vec2, double);
bool QT_Subdivide(QuadTree*);
bool QT_Insert(QuadTree*, Entity*);
void QT_FreeTree(QuadTree*);
void QT_FreeNode(QuadTree*);
void QT_UpdateCOM(QuadTree*);
void QT_BarnesHut(QuadTree*,Entity*);
bool QT_HandleCollisions(QuadTree*);

#endif
