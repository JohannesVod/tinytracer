#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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

void read_obj_file(const char *filename, Mesh *mesh) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    size_t vertex_capacity = 10;
    size_t triangle_capacity = 10;
    mesh->vertices = malloc(vertex_capacity * sizeof(Vec3));
    mesh->triangles = malloc(triangle_capacity * sizeof(Triangle));
    mesh->vertex_count = 0;
    mesh->triangle_count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            if (mesh->vertex_count >= vertex_capacity) {
                vertex_capacity *= 2;
                mesh->vertices = realloc(mesh->vertices, vertex_capacity * sizeof(Vec3));
            }
            Vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            mesh->vertices[mesh->vertex_count++] = v;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (mesh->triangle_count >= triangle_capacity) {
                triangle_capacity *= 2;
                mesh->triangles = realloc(mesh->triangles, triangle_capacity * sizeof(Triangle));
            }
            int v1, v2, v3;
            sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &v1, &v2, &v3);
            Triangle t;
            t.v1 = mesh->vertices[v1 - 1];
            t.v2 = mesh->vertices[v2 - 1];
            t.v3 = mesh->vertices[v3 - 1];
            mesh->triangles[mesh->triangle_count++] = t;
        }
    }
    fclose(file);
}

void free_mesh(Mesh *mesh) {
    free(mesh->vertices);
    free(mesh->triangles);
}

int ray_intersects_triangle_optimized(float rox, float roy, float roz,
                                      float rdx, float rdy, float rdz,
                                      float v0x, float v0y, float v0z,
                                      float v1x, float v1y, float v1z,
                                      float v2x, float v2y, float v2z,
                                      float *t_out, float *u_out, float *v_out) 
{
    float e1x = v1x - v0x;
    float e1y = v1y - v0y;
    float e1z = v1z - v0z;
    
    float e2x = v2x - v0x;
    float e2y = v2y - v0y;
    float e2z = v2z - v0z;
    
    float hx = rdy * e2z - rdz * e2y;
    float hy = rdz * e2x - rdx * e2z;
    float hz = rdx * e2y - rdy * e2x;
    
    float a = e1x * hx + e1y * hy + e1z * hz;
    
    if (fabs(a) < 1e-6) {
        return 0; // Parallel to triangle plane
    }
    
    float f = 1.0 / a;
    
    float sx = rox - v0x;
    float sy = roy - v0y;
    float sz = roz - v0z;
    
    float u = f * (sx * hx + sy * hy + sz * hz);
    
    if (u < 0.0 || u > 1.0) {
        return 0;
    }
    
    float qx = sy * e1z - sz * e1y;
    float qy = sz * e1x - sx * e1z;
    float qz = sx * e1y - sy * e1x;
    
    float v = f * (rdx * qx + rdy * qy + rdz * qz);
    
    if (v < 0.0 || u + v > 1.0) {
        return 0;
    }
    
    float t = f * (e2x * qx + e2y * qy + e2z * qz);
    
    if (t > 1e-6) { // ray intersection
        *t_out = t;
        *u_out = u;
        *v_out = v;
        return 1;
    }
    return 0;
}

int ray_intersects_triangle(Ray *ray, Triangle *triangle, Vec3 *out) 
{
    float t, u, v;
    int hit = ray_intersects_triangle_optimized(ray->origin.x, ray->origin.y, ray->origin.z,
                                                ray->direction.x, ray->direction.y, ray->direction.z,
                                                triangle->v1.x, triangle->v1.y, triangle->v1.z,
                                                triangle->v2.x, triangle->v2.y, triangle->v2.z,
                                                triangle->v3.x, triangle->v3.y, triangle->v3.z,
                                                &t, &u, &v);
    if (hit) {
        out->x = t;
        out->y = u;
        out->z = v;
    }
    return hit;
}

int ray_intersects_mesh(Ray *ray, Mesh *mesh, Vec3 *out) {
    for (size_t i = 0; i < mesh->triangle_count; i++) {
        if (ray_intersects_triangle(ray, &mesh->triangles[i], out)) {
            return 1;
        }
    }
    return 0;
}

int screen2CameraDir(Camera *cam, int screenPos_x, int screenPos_y, Vec3 *result) {
    Vec3 cam_coor = {
        (float)screenPos_x / (float)cam->height,
        (float)screenPos_y / (float)cam->height,
        0
    };
    Vec3 center_shift = {
        -((float)cam->width / (float)cam->height) * 0.5, -0.5, -cam->focal_length
    };
    vec3_add(&cam_coor, &center_shift, result);
    return 1;
}

#endif // MESH_H
