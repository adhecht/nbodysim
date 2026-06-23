/*
 *  vector.h
 */

#ifndef _VECTOR_H
#define _VECTOR_H 1

struct Vec2 {
    double x, y;
};

Vec2 add(Vec2, Vec2);
Vec2 sub(Vec2, Vec2);
Vec2 scale(Vec2, double);
double dot_product(Vec2, Vec2);
double length(Vec2);
Vec2 normalize(Vec2);
double euclidean_distance(Vec2, Vec2);
double square_euclidean_distance(Vec2, Vec2);

void print_vector(Vec2);
#endif
