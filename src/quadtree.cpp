/*
 *  quadtree.cpp
 */

#include "quadtree.h"
#include "bounding_box.h"
#include "vector.h"
#include "debug.h"
#include "physics.h"

#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <math.h>

QuadTree* QT_Create(Vec2 center, double half_dim) {
    QuadTree *ptr = (QuadTree*)malloc(sizeof(QuadTree));
    g_node_count++;
    memset(ptr, 0, sizeof(QuadTree));
    if (ptr) {
        ptr->bounds.center = center;
        ptr->bounds.half_dim = half_dim;
        ptr->is_leaf = true;
    }
    return ptr;
}

void QT_FreeNode(QuadTree *node) {
    if (node) {
        free(node);
        g_node_count--;
        node = NULL;
    }
    return;
}

void QT_FreeTree(QuadTree *root) {
    if (root) {
        if (root->child[0]) {
            for (ptrdiff_t i = 0; i < 4; i++) {
                QT_FreeTree(root->child[i]);
                root->child[i] = nullptr;
            }
        }
        QT_FreeNode(root);
    }
    return;
}

bool RegionContainsPoint(AABB region, Vec2 point) {
    Vec2 center = region.center;
    double half_dim = region.half_dim;
    return (((center.x - half_dim) <= point.x) &&
            ((center.x + half_dim) > point.x) &&
            ((center.y - half_dim) <= point.y) &&
            ((center.y + half_dim) > point.y));
}

bool QT_Subdivide(QuadTree *node) {
    if (node) {
        if (!node->child[0]) {
            Vec2 center = node->bounds.center;
            double half_dim = node->bounds.half_dim / 2.0;
            node->child[0] = QT_Create({center.x - half_dim, center.y + half_dim}, half_dim);
            node->child[1] = QT_Create({center.x + half_dim, center.y + half_dim}, half_dim);
            node->child[2] = QT_Create({center.x - half_dim, center.y - half_dim}, half_dim);
            node->child[3] = QT_Create({center.x + half_dim, center.y - half_dim}, half_dim);
            for (ptrdiff_t i = 0; i < QT_NODE_MAX; i++) {
                for (ptrdiff_t j = 0; j < 4; j++) {
                    if (QT_Insert(node->child[j], node->entities[i])) {
                        break;
                    }
                }
            }
            node->is_leaf = false;
            node->n_entities = 0;
            return true;
        }
    }
    return false;
}


bool QT_Insert(QuadTree *root, Entity *entity) {
    if (RegionContainsPoint(root->bounds, entity->position)) {
        if (!root->child[0] && (root->n_entities < QT_NODE_MAX)) {
            root->entities[root->n_entities] = entity;
            root->n_entities++;
            return true;
        }
        if (!root->child[0]) {
            QT_Subdivide(root);
        }
        for (ptrdiff_t i = 0; i < 4; i++) {
            if (QT_Insert(root->child[i], entity)) {
                break;
            }
        }
        return true;
    }
    return false;
}

void QT_UpdateCOM(QuadTree *root) {
    if (root) {
        root->center_of_mass = {0.0, 0.0};
        root->total_mass = 0.0;
        if (root->child[0]) {
            for (ptrdiff_t i = 0; i < 4; i++) {
                QT_UpdateCOM(root->child[i]);
            } 
            for (ptrdiff_t i = 0; i < 4; i++) {
                Vec2 child_position = root->child[i]->center_of_mass;
                double child_mass = root->child[i]->total_mass;
                root->total_mass += child_mass;
                if (child_mass > 0.0) {
                    root->center_of_mass = add(root->center_of_mass, scale(child_position, child_mass));
                }
            }
            root->center_of_mass = scale(root->center_of_mass, 1.0 / root->total_mass);
        } else { /* leaf node */
            for (ptrdiff_t i = 0; i < root->n_entities; i++) {
                Vec2 entity_position = root->entities[i]->position;
                double entity_mass = root->entities[i]->mass;
                root->total_mass += entity_mass;
                root->center_of_mass = add(root->center_of_mass, scale(entity_position, entity_mass));
            } 
            if (root->total_mass > 0.0) {
                root->center_of_mass = scale(root->center_of_mass, 1.0 / root->total_mass);
            }
        }
    }
}

void QT_BarnesHut(QuadTree *node, Entity *entity) {
    double threshold = 10.0; /* threshold = 0.0 -> direct sum */
    if (node) {
        if (node->total_mass > 0) {
            double distance = euclidean_distance(node->center_of_mass, entity->position);
            double node_width = node->bounds.half_dim * 2.0;
            double ratio = node_width / (distance);
            if (ratio <= threshold && !node->is_tree_root) {
                Entity node_barycenter = {};
                node_barycenter.position = node->center_of_mass;
                node_barycenter.mass = node->total_mass;
                Gravitate(entity, &node_barycenter); 
            } else {
                if (node->is_leaf) {
                    for (ptrdiff_t i = 0; i < node->n_entities; i++) {
                        if (entity != node->entities[i] && node->entities[i]->is_active) {
                            Gravitate(entity, node->entities[i]);
                        }
                    }
                } else {
                    for (ptrdiff_t i = 0; i < 4; i++) {
                        QT_BarnesHut(node->child[i], entity);
                    }
                }
            }
        }
    }
}

bool CheckCollision(Entity *A, Entity *B) {
    double square_distance = pow(B->position.x - A->position.x, 2) + pow(B->position.y - A->position.y, 2);
    double radius = A->radius + B->radius;
    double radius_squared = radius * radius;
    if (square_distance <= radius_squared) {
        return true;
    }
    return false;
}

bool QT_HandleCollisions(QuadTree *node) {
    if (node->child[0]) {
        for (ptrdiff_t i = 0; i < 4; i++) {
            if (QT_HandleCollisions(node->child[i])) {
                return true;
            }
        }
    } else { /* leaf node */
        for (ptrdiff_t i = 0; i < node->n_entities; i++) {
            Entity *A = node->entities[i];
            for (ptrdiff_t j = 0; j < node->n_entities; j++) {
                if (i == j) {
                    break;
                }
                Entity *B = node->entities[j];
                if (CheckCollision(A, B)) {
                    double mass = A->mass + B->mass;
                    Vec2 position = scale(add(scale(A->position, A->mass), scale(B->position, B->mass)), 1/mass); /* mass-averaged position */
                    Vec2 momentum = add(scale(A->velocity, A->mass), scale(B->velocity, B->mass));
                    Vec2 velocity = scale(momentum, 1/mass);
                    A->position = position;
                    A->velocity = velocity;
                    A->mass = mass;
                    A->radius = log10(A->mass) + 1;
                    B->is_active = false;
                }
            }
        }
    }
    return false;
}
