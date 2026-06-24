
#include "quadtree.h"
#include "vector.h"
#include "entity.h"
#include "physics.h"
#include "camera.h"

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <random>
#include <math.h>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>

struct Window {
    int width, height;
};

struct App {
    ALLEGRO_DISPLAY *display;
    ALLEGRO_TIMER *timer;
    ALLEGRO_EVENT_QUEUE *queue;
    Window window;
    float target_fps;
};

bool g_allegro_initialized = false;

int init_app(App *app, int width, int height, float fps) {
    if (!g_allegro_initialized) { 
        al_init();
        g_allegro_initialized = true;
    }
    if (app) {
        if (!app->display) {
            app->display = al_create_display(width, height);
            app->window.width = width;
            app->window.height = height;
        }
        if (!app->timer) {
            app->timer = al_create_timer(1.0f / fps);
            app->target_fps = fps;
        }
        if (!app->queue) {
            app->queue = al_create_event_queue();
        }
    }
    return 0;
}

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

double rotation_curve(double distance) {
    return log10(distance*2);
}

static bool g_render_COM = true;
static bool g_render_tree = false;

bool g_pressed_keys[ALLEGRO_KEY_MAX] = {};

double FindMaxDistance(Entity *entities, ptrdiff_t n_entities) {
    double max_distance = 0.0;

    for (ptrdiff_t i = 0; i < n_entities; i++) {
        for (ptrdiff_t j = i; j < n_entities; j++) {
            if (i == j) {
                continue;
            }
            double distance = euclidean_distance(entities[i].position, entities[j].position);
            if (distance > max_distance) {
                max_distance = distance;
            }
        }
    }

    return max_distance;
}

int main(int argc, char *argv[]) {

    /* initialize allegro */
    if (!al_init()) {
        printf("Unable to initialize Allegro5. Exiting now.\n");
        exit(EXIT_FAILURE);
    }

    if (!al_init_primitives_addon()) {
        printf("Unable to initialize Allegro5 primitives addon. Exiting now.\n");
        exit(EXIT_FAILURE);
    }

    if (!al_install_keyboard()) {
        printf("Warning: unable to install keyboard.\n");
    }

    const int screen_height = 640;
    const int screen_width = 640;

    ALLEGRO_DISPLAY *display = al_create_display(screen_width, screen_height);
    if (!display) {
        printf("Unable to create display. Exiting now.\n");
        exit(EXIT_FAILURE);
    }

    const double target_fps = 30.0;
    ALLEGRO_TIMER *timer = al_create_timer(1/target_fps);
    if (!timer) {
        printf("Unable to create timer. Exiting now.\n");
        al_destroy_display(display);
        exit(EXIT_FAILURE);
    }

    ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
    if (!queue) {
        printf("Unable to create event queue. Exiting now.\n");
        al_destroy_timer(timer);
        al_destroy_display(display);
        exit(EXIT_FAILURE);
    }

    /* register event sources */
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_keyboard_event_source());

    /* initialize simulation */
    const double world_width = (double)screen_width*4;
    const double world_height = (double)screen_height*4;

    Camera camera = {};
    camera.position.x = -3*world_width/8;
    camera.position.y = -3*world_height/8;
    camera.scale = 10;
    camera.zoom_speed = 0.1;
    camera.velocity.x = 50;
    camera.velocity.y = 50;

    
    /* initialize random number generator */
    std::random_device dev;
    std::mt19937 rng(dev());
    const double pi = acos(-1);
    std::uniform_real_distribution<double> random_angle(0, 2*pi);
    std::uniform_real_distribution<double> random_radius(50, world_width/3);
    std::uniform_real_distribution<double> random_velocity(-0.1,0.4);
    std::uniform_real_distribution<double> random_mass(0.2,3);

    /* initialize quadtree root node */
    Vec2 center = {world_width/2.0, world_width/2.0};
    QuadTree *root = QT_Create(center, world_width/2.0);
    root->is_tree_root = true;

    /* create entities */
    ptrdiff_t n_entities = 2500;
    Entity *entities = (Entity*)malloc(sizeof(Entity)*n_entities);
    //memset(entities, 0, sizeof(Entity)*n_entities);
    
    /* set up a black hole at center */
    Entity *entity = &entities[0];
    entity->position = center;
    entity->velocity = {};
    entity->acceleration = {};
    entity->is_active = true;
    entity->mass = 1000000;
    entity->radius = log10(entity->mass) + 1;

    /* randomly generate rest of galaxy */
    for (ptrdiff_t i = 1; i < n_entities; i++) {
        Entity *entity = &entities[i];

        /* calculate position in polar coordinates and convert to cartesian */
        double angle = random_angle(dev);
        double radius = random_radius(dev);
        entity->position.x = (radius * cos(angle)) + center.x;
        entity->position.y = (radius * sin(angle)) + center.y;

        /* compute velocity based on rotation curve */
        Vec2 velocity = {};
        velocity.x = -sin(angle);
        velocity.y = cos(angle);

        double magnitude = rotation_curve(euclidean_distance(entity->position, center));
        entity->velocity = scale(velocity, magnitude);


        entity->acceleration = {0.0, 0.0};

        entity->mass = random_mass(dev);
        entity->radius = log10(entity->mass) + 1; /* minimum radius is 5 */
        entity->is_active = true;

        QT_Insert(root, entity); /* add to quadtree */
    }

    QT_UpdateCOM(root);

    QT_HandleCollisions(root);

    double dt = 0.5;

    bool redraw = false;
    bool is_running = true;
    al_start_timer(timer);
    while (is_running) {
        ALLEGRO_EVENT event;
        al_wait_for_event(queue, &event);

        /* handle keyboard events */
        if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (event.keyboard.keycode) {
                case ALLEGRO_KEY_T:
                    g_render_tree = !g_render_tree;
                    break;
                case ALLEGRO_KEY_C:
                    g_render_COM = !g_render_COM;
                    break;
                default:
                    g_pressed_keys[event.keyboard.keycode] = true;
                    break;
            }
        }

        if (event.type == ALLEGRO_EVENT_KEY_UP) {
            switch (event.keyboard.keycode) {
                default:
                    g_pressed_keys[event.keyboard.keycode] = false;
                    break;
            }
        }

        if (event.type == ALLEGRO_EVENT_TIMER) {
            /* calculate gravitational acceleratoin */
            for (ptrdiff_t i = 0; i < n_entities; i++) {
                Entity *entity = &entities[i];
                entity->acceleration = {0.0, 0.0};
                QT_BarnesHut(root, entity);
            }

            /* update state */
            for (ptrdiff_t i = 0; i < n_entities; i++) {
                Entity *A = &entities[i];
                A->velocity = add(A->velocity, scale(A->acceleration, dt));
                A->position = add(A->position, scale(A->velocity, dt));
            }


            /* re-build the quadtree */
            QT_FreeTree(root);
            root = QT_Create(center, world_width/2);
            root->is_tree_root = true;
            for (ptrdiff_t i = 0; i < n_entities; i++) {
                if (entities[i].is_active) {
                    QT_Insert(root, &entities[i]);
                }
            }

            /* update the center of mass */
            QT_UpdateCOM(root);

            
            

            /* check collisions */
            //QT_HandleCollisions(root);  
            
            /* handle inputs */
            if (g_pressed_keys[ALLEGRO_KEY_W]) {
                camera.position.y += camera.velocity.y * dt;
            }

            if (g_pressed_keys[ALLEGRO_KEY_S]) {
                camera.position.y -= camera.velocity.y * dt;
            }

            if (g_pressed_keys[ALLEGRO_KEY_A]) {
                camera.position.x += camera.velocity.x * dt;
            }

            if (g_pressed_keys[ALLEGRO_KEY_D]) {
                camera.position.x -= camera.velocity.x * dt;
            }

            if (g_pressed_keys[ALLEGRO_KEY_Q]) {
                camera.scale += camera.zoom_speed * dt;
            }

            if (g_pressed_keys[ALLEGRO_KEY_E]) {
                if (camera.scale > 0.01) {
                    camera.scale -= camera.zoom_speed * dt;
                } else {
                    camera.scale = 0.01;
                }
            }

            redraw = true;
        }
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            is_running = false;
        }

        

        if (redraw) {

                

            al_clear_to_color(al_map_rgb(0,0,0));

            for (ptrdiff_t i = 0; i < n_entities; i++) {
                if (entities[i].is_active) {
                    render_particle(&camera, &entities[i]);
                    render_velocity(&camera, &entities[i]);
                    render_acceleration(&camera, &entities[i]);
                }
            }

            if (g_render_COM) {
                al_draw_filled_circle(root->center_of_mass.x, root->center_of_mass.y, 5, al_map_rgb(255,0,0));
            }

            if (g_render_tree) {
                render_quadtree(&camera, root);
            }
            al_flip_display();
            redraw = false;
        }

    }

    /* clean up and exit */

    QT_FreeTree(root);

    al_destroy_display(display);
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);

    exit(EXIT_SUCCESS);
}
