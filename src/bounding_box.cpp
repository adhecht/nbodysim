#include "bounding_box.h"
#include "vector.h"

bool RegionContainsPoint(AABB region, Vec2 point) {
    Vec2 center = region.center;
    double half_dim = region.half_dim;
    return (((center.x - half_dim) <= point.x) &&
            ((center.x + half_dim) > point.x) &&
            ((center.y - half_dim) <= point.y) &&
            ((center.y + half_dim) > point.y));
}
