/*
 *      render.h
 */

#ifndef _RENDER_H
#define _RENDER_H 1

#include "camera.h"
#include "entity.h"
#include "quadtree.h"

void render_particle(Camera*, Entity*);
void render_quadtree(Camera*, QuadTree*);
void render_velocity(Camera*, Entity*);
void render_acceleration(Camera*, Entity*);

#endif
