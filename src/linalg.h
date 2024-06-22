#ifndef LINALG_H
#define LINALG_H
#include <math.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    int x, y, z;
} Vec3Int;

typedef struct {
    Vec3 origin, normal;
} Plane;

typedef struct {
    Vec3 origin, direction;
} Ray;

void vec3_subtract(Vec3 *a, Vec3 *b, Vec3 *result) {
    result->x = a->x - b->x;
    result->y = a->y - b->y;
    result->z = a->z - b->z;
}

void vec2_subtract(Vec2 *a, Vec2 *b, Vec2 *result) {
    result->x = a->x - b->x;
    result->y = a->y - b->y;
}

void vec2_add(Vec2 *a, Vec2 *b, Vec2 *result) {
    result->x = a->x + b->x;
    result->y = a->y + b->y;
}

void vec2_scale(Vec2 *a, float t, Vec2 *result) {
    result->x = a->x * t;
    result->y = a->y * t;
}

void vec2_copy(Vec2 *v, Vec2 *result) {
    result->x = v->x;
    result->y = v->y;
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

void vec3_mul(Vec3 *a, Vec3 *b, Vec3 *result) {
    result->x = a->x * b->x;
    result->y = a->y * b->y;
    result->z = a->z * b->z;
}

void vec3_abs(Vec3 *a, Vec3 *result) {
    result->x = fabs(a->x);
    result->y = fabs(a->y);
    result->z = fabs(a->z);
}

void vec3_inverse(Vec3 *a, Vec3 *result){
    result->x = 1/a->x;
    result->y = 1/a->y;
    result->z = 1/a->z;
}

void vec3_invert(Vec3 *a, Vec3 *result){
    result->x = -a->x;
    result->y = -a->y;
    result->z = -a->z;
}

void vec3_copy(Vec3 *v, Vec3 *result) {
    result->x = v->x;
    result->y = v->y;
    result->z = v->z;
}

void vec3_floor(Vec3 *v, Vec3 *result) {
    result->x = floor(v->x);
    result->y = floor(v->y);
    result->z = floor(v->z);
}

void vec3_ceil(Vec3 *v, Vec3 *result) {
    result->x = floor(v->x+1);
    result->y = floor(v->y+1);
    result->z = floor(v->z+1);
}

void vec3_round(Vec3 *v, Vec3 *result) {
    result->x = roundf(v->x);
    result->y = roundf(v->y);
    result->z = roundf(v->z);
}

void vec3_2int(Vec3 *v, Vec3Int *result){
    result->x = (int)(v->x+0.001);
    result->y = (int)(v->y+0.001);
    result->z = (int)(v->z+0.001);
}

/* adds small value to every coordinate that is zero*/
void vec3_fix(Vec3 *v){
    if(v->x == 0){v->x += 0.0000001f;}
    if(v->y == 0){v->y += 0.0000001f;}
    if(v->z == 0){v->z += 0.0000001f;}
}

float vec3_magnitude(Vec3 *v) {
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

float randFloat(){
    float x = (float)rand()/(float)(RAND_MAX);
    return x;
}

void vec3_normalize(Vec3 *v, Vec3 *result) {
    float magnitude = vec3_magnitude(v);
    if (magnitude > 0) {
        vec3_scale(result, 1/magnitude, result);
    } else {
        // If the magnitude is zero, return a zero vector
        result->x = 0;
        result->y = 0;
        result->z = 0;
    }
}

/* return random point on unit sphere. Randomized algorithm. TODO: try out faster functions */
Vec3 rand_unit(){
    Vec3 curr;
    for (size_t i = 0; i < 100; i++)
    {
        curr.x = (randFloat()-0.5)*2;
        curr.y = (randFloat()-0.5)*2;
        curr.z = (randFloat()-0.5)*2;
        float mag_squared = vec3_dot(&curr, &curr);
        if (mag_squared <= 1 && mag_squared != 0){
            vec3_scale(&curr, 1/sqrt(mag_squared), &curr);
            return curr;
        }
    }
    Vec3 ret_vec = {1, 0, 0}; 
    return ret_vec;
}

Vec3 rand_hemi_vec(Vec3 *normal){
    Vec3 rand = rand_unit();
    if (vec3_dot(normal, &rand) < 0){
        vec3_invert(&rand, &rand);
    }
    return rand;
}

/* Returns random vector along normal using lambertian reflection*/
Vec3 rand_lambertian(Vec3 *normal){
    Vec3 rand = rand_unit();
    vec3_add(&rand, normal, &rand);
    vec3_normalize(&rand, &rand);
    return rand;
}

int get_intersection_point(Plane *p, Ray *r, Vec3 *result) {
    if (fabs(vec3_dot(&r->direction, &p->normal)) <= 1e-6) {
        return 0; // interpreted as no intersection
    }
    vec3_subtract(&p->origin, &r->origin, result);
    float t = vec3_dot(result, &p->normal) / vec3_dot(&r->direction, &p->normal);
    if (t <= 0) {
        return 0;
    }
    vec3_scale(&r->direction, t, result);
    vec3_add(&r->origin, result, result);
    return 1;
}

#endif // VEC3_H