#include "physics.h"
#include "vector.h"

static const double GRAVITATIONAL_CONSTANT = 4.30091727063e-3; /* pc*Ms*(km/s)^2 */
static const double EPSILON = 5.0;

double GravitationalForce(Entity *A, Entity *B) {
    const double G = GRAVITATIONAL_CONSTANT;
    double distance = euclidean_distance(A->position, B->position);
    distance += EPSILON; /* add an epsilon to prevent small distances from blowing up the force */
    return (double) (G*A->mass*B->mass)/(distance*distance);
}

void Gravitate(Entity *A, Entity *B) {
    double force = GravitationalForce(A, B);
    Vec2 acceleration_vector = normalize(sub(B->position, A->position));
    Vec2 scaled_acceleration = scale(acceleration_vector, force/A->mass);
    A->acceleration = add(A->acceleration, scaled_acceleration);
    return;
}
