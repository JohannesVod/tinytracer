// vec3.h
#ifndef VEC3_H
#define VEC3_H

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    Vec3 origin, normal;
} Plane;

typedef struct {
    Vec3 origin, direction;
} Ray;

void vec3_subtract(Vec3 *a, Vec3 *b, Vec3 *result);
void vec3_cross(Vec3 *a, Vec3 *b, Vec3 *result);
float vec3_dot(Vec3 *a, Vec3 *b);
void vec3_scale(Vec3 *a, float t, Vec3 *result);
void vec3_add(Vec3 *a, Vec3 *b, Vec3 *result);
int get_intersection_point(Plane *p, Ray *r, Vec3 *result);
void vec3_copy(Vec3 *v, Vec3 *result);

#endif // VEC3_H