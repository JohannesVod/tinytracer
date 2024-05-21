#include "linalg.h"

Vec3 vec3_subtract(Vec3 a, Vec3 b) {
    Vec3 result = { a.x - b.x, a.y - b.y, a.z - b.z };
    return result;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    Vec3 result = { 
        a.y * b.z - a.z * b.y, 
        a.z * b.x - a.x * b.z, 
        a.x * b.y - a.y * b.x 
    };
    return result;
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_scale(Vec3 a, float t) {
    Vec3 result = { a.x * t, a.y * t, a.z * t };
    return result;
}

Vec3 vec3_add(Vec3 a, Vec3 b) {
    Vec3 result = { a.x + b.x, a.y + b.y, a.z + b.z };
    return result;
}