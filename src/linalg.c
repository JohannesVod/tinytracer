#include "linalg.h"
#include <math.h>

void vec3_subtract(Vec3 *a, Vec3 *b, Vec3 *result) {
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
}

void vec3_cross(Vec3 *a, Vec3 *b, Vec3 *result) {
    result->x = a->y * b->z - a->z * b->y;
    result->y = a->z * b->x - a->x * b->z;
    result->z = a->x * b->y - a->y * b->x;
}

float vec3_dot(Vec3 *a, Vec3 *b) {
    return a->x * b->x + a->y * b->y + a->z * b->z;
}

void vec3_scale(Vec3 *a, float t, Vec3 *result) {
    result->x = a->x * t;
    result->y = a->y * t;
    result->z = a->z * t;
}

void vec3_add(Vec3 *a, Vec3 *b, Vec3 *result) {
    result->x = a->x + b->x;
    result->y = a->y + b->y;
    result->z = a->z + b->z;
}

void vec3_copy(Vec3 *v, Vec3 *result){
    result->x = v->x;
    result->y = v->y;
    result->z = v->z;
}

int get_intersection_point(Plane *p, Ray *r, Vec3 *result){
    // check if ray is parallel to plane first:
    if (fabs(vec3_dot(&r->direction, &p->normal)) <= 1e-6){
        return 0; // interpreted as no intersection
    }
    vec3_subtract(&p->origin, &r->origin, result);
    float t = vec3_dot(result, &p->normal)/vec3_dot(&r->direction, &p->normal);
    if (t <= 0){
        return 0;
    }
    vec3_scale(&r->direction, t, result);
    vec3_add(&r->origin, result, result);
    return 1;
}