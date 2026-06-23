/*
 *  entity.h
 */

#ifndef _ENTITY_H
#define _ENTITY_H 1

#include "vector.h"

struct Entity {
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;
    double mass;
    double radius;
    bool is_active;
};

#endif
