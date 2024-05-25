// mesh.h
#ifndef MESH_H
#define MESH_H

#include "linalg.h"

typedef struct {
    Vec3 v1, v2, v3;
} Triangle;

typedef struct {
    Vec3 *vertices;
    size_t vertex_count;
    Triangle *triangles;
    size_t triangle_count;
} Mesh;

typedef struct {
    Vec3 position, rotation;
    int width;
    int height;
    float focal_length;
} Camera;

void read_obj_file(const char *filename, Mesh *mesh);
void free_mesh(Mesh *mesh);
int ray_intersects_triangle(Ray *ray, Triangle *triangle, Vec3 *out); 
int ray_intersects_mesh(Ray *ray, Mesh *mesh, Vec3 *out);
int screen2CameraDir(Camera *cam, int screenPos_x, int screenPos_y, Vec3 *result);

#endif // MESH_H