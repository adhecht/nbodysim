#include "vector.h"

#include <cstdio>
#include <math.h>

Vec2 add(Vec2 A, Vec2 B) {
    Vec2 C = {};
    C.x = A.x + B.x;
    C.y = A.y + B.y;
    return C;
}

Vec2 sub(Vec2 A, Vec2 B) {
    Vec2 C = {};
    C.x = A.x - B.x;
    C.y = A.y - B.y;
    return C;
}

Vec2 scale(Vec2 A, double s) {
    Vec2 C = {};
    C.x = s * A.x;
    C.y = s * A.y;
    return C;
}

double dot_product(Vec2 A, Vec2 B) {
    return (double)(A.x*B.x + A.y*B.y);
}

double length(Vec2 A) {
    return (double)sqrt(pow(A.x,2) + pow(A.y,2));
}

Vec2 normalize(Vec2 A) {
    double l = length(A);
    Vec2 C = scale(A, 1.0 / l);
    return C;
}

double euclidean_distance(Vec2 A, Vec2 B) {
    return (double)sqrt(pow(B.x-A.x,2)+pow(B.y-A.y,2));
}

double square_euclidean_distance(Vec2 A, Vec2 B) {
    return (double)(pow(B.x-A.x,2)+pow(B.y-A.y,2));
}

void print_vector(Vec2 A) {
    printf("<%f, %f>\n", A.x, A.y);
    return;
}
