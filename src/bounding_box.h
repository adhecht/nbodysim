/*
 *  bounding_box.h
 */

#ifndef _BOUNDING_BOX_H
#define _BOUNDING_BOX_H 1

#include "vector.h"

struct AABB {
    Vec2 center;
    double half_dim;
};

bool RegionContainsPoint(AABB, Vec2);

#endif
