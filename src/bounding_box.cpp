#include "bounding_box.h"
#include "vector.h"

bool ContainsPoint(AABB bounds, Vec2 point) {
    return (((bounds.center.x + bounds.half_dim) > point.x) &&
            ((bounds.center.x - bounds.half_dim) <= point.x) &&
            ((bounds.center.y + bounds.half_dim) > point.y) &&
            ((bounds.center.y - bounds.half_dim) <= point.y));
}
