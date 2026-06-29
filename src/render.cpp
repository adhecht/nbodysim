#include "render.h"

void render_particle(Camera *camera, Entity *particle) {
    if (camera) {
        if (particle) {
            float x = particle->position.x + camera->position.x;
            float y = particle->position.y + camera->position.y;
            x = x * (1/camera->scale);
            y = y * (1/camera->scale);
            al_draw_filled_circle(x, y, particle->radius, al_map_rgb(255, 255, 255));
        }
    }
}

void render_quadtree(Camera *camera, QuadTree *node) {
    if (node->child[0]) {
        for (ptrdiff_t i = 0; i < 4; i++) {
            render_quadtree(camera, node->child[i]);
        }
    } else {
        float x0, y0, x1, y1;
        Vec2 center = node->bounds.center;
        center = add(center, camera->position);
        Vec2 upper_left = {center.x - node->bounds.half_dim, center.y + node->bounds.half_dim};
        Vec2 lower_right = {center.x + node->bounds.half_dim, center.y - node->bounds.half_dim};

        upper_left = scale(upper_left, 1 / camera->scale);
        lower_right = scale(lower_right, 1 / camera->scale);

        x0 = upper_left.x;
        x1 = lower_right.x;
        y0 = upper_left.y;
        y1 = lower_right.y;
 
        al_draw_rectangle(x0,y0,x1,y1,al_map_rgb(0,255,0), 2);
    }
}

void render_velocity(Camera *camera, Entity *entity) {
    Vec2 position = add(entity->position, camera->position);
    Vec2 transformed_vector = add(position, entity->velocity);
    float x0, x1, y0, y1;
    x0 = position.x * (1 / camera->scale);
    y0 = position.y * (1 / camera->scale);
    x1 = transformed_vector.x * (1 / camera->scale);
    y1 = transformed_vector.y * (1 / camera->scale);
    al_draw_line(x0,y0,x1,y1,al_map_rgb(0,0,255),2);
}

void render_acceleration(Camera *camera, Entity *entity) {
    Vec2 position = add(entity->position, camera->position);
    Vec2 transformed_vector = add(position, entity->acceleration);
    float x0, x1, y0, y1;
    x0 = position.x * (1 / camera->scale);
    y0 = position.y * (1 / camera->scale);
    x1 = transformed_vector.x * (1 / camera->scale);
    y1 = transformed_vector.y * (1 / camera->scale);
    al_draw_line(x0,y0,x1,y1,al_map_rgb(255,120,255),2);
}

