#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "linalg.h"

typedef struct {
    Vec3 v1, v2, v3;
    Vec3 normal;
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

void calculate_normal(Triangle *triangle) {
    Vec3 e1, e2;
    vec3_subtract(&triangle->v2, &triangle->v1, &e1);
    vec3_subtract(&triangle->v3, &triangle->v1, &e2);
    vec3_cross(&e1, &e2, &triangle->normal);
    vec3_normalize(&triangle->normal, &triangle->normal);
}

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
            calculate_normal(&t);
            mesh->triangles[mesh->triangle_count++] = t;
        }
    }
    fclose(file);
}

void free_mesh(Mesh *mesh) {
    free(mesh->vertices);
    free(mesh->triangles);
}

int ray_intersects_triangle(Ray *ray, Triangle *triangle, Vec3 *out) {
    const float epsilon = 1e-6;
    Vec3 e1, e2, e2_cross_raydir, b_cross_e1, b;
    vec3_subtract(&triangle->v2, &triangle->v1, &e1);
    vec3_subtract(&triangle->v3, &triangle->v1, &e2);
    vec3_cross(&ray->direction, &e2, &e2_cross_raydir);
    float det = vec3_dot(&e1, &e2_cross_raydir);
    if (det <= epsilon && -det <= epsilon) {
        return 0; // no solution because ray is parallel to triangle plane
    }
    float inv_det = 1.0 / det; // calculate once because div is expensive
    vec3_subtract(&ray->origin, &triangle->v1, &b);

    float u = inv_det * vec3_dot(&e2_cross_raydir, &b); // u
    if (u < 0.0 || u > 1.0) {
        return 0;
    }
    vec3_cross(&b, &e1, &b_cross_e1);
    float v = inv_det * vec3_dot(&ray->direction, &b_cross_e1); // v
    if (v < 0.0 || v + u > 1.0) {
        return 0;
    }
    float t = inv_det * vec3_dot(&e2, &b_cross_e1); // t
    if (t >= epsilon) {
        out->x = t;
        out->y = u;
        out->z = v;
        return 1;
    }
    return 0;
}

float reflect(Ray *ray, Triangle *triangle, Vec3 *out){
    float dot_prod = vec3_dot(&ray->direction, &triangle->normal);
    vec3_scale(&triangle->normal, -2*dot_prod, out);
    vec3_add(&ray->direction, out, out);
    return dot_prod;
}

float reflectMesh(Ray *ray, Mesh *mesh, Vec3 *out) {
    for (size_t i = 0; i < mesh->triangle_count; i++) {
        if (ray_intersects_triangle(ray, &mesh->triangles[i], out)) {
            return reflect(ray, &mesh->triangles[i], out);
        }
    }
    return 0;
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
    vec3_normalize(result, result);
    return 1;
}

#endif // MESH_H
