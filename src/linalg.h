// vec3.h
#ifndef VEC3_H
#define VEC3_H

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float x, y;
} Vec2;

Vec3 vec3_subtract(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_scale(Vec3 a, float t);
Vec3 vec3_add(Vec3 a, Vec3 b);

#endif // VEC3_H