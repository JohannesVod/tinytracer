#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "linalg.h"

typedef struct {
    Vec3 N, N1, N2; // planes
    float d, d1, d2; // distances of planes from origin
} Triangle;

typedef struct {
    Vec3 *vertices;
    Vec3 *normals;
    size_t vertex_count;
    size_t normal_count;
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
    size_t normal_capacity = 10;
    size_t triangle_capacity = 10;
    mesh->vertices = malloc(vertex_capacity * sizeof(Vec3));
    mesh->normals = malloc(normal_capacity * sizeof(Vec3));
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
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (mesh->normal_count >= normal_capacity) {
                normal_capacity *= 2;
                mesh->normals = realloc(mesh->normals, normal_capacity * sizeof(Vec3));
            }
            Vec3 n;
            sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
            mesh->normals[mesh->normal_count++] = n;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (mesh->triangle_count >= triangle_capacity) {
                triangle_capacity *= 2;
                mesh->triangles = realloc(mesh->triangles, triangle_capacity * sizeof(Triangle));
            }
            int v1I, v2I, v3I;
            int vn;
            sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%d", &v1I, &v2I, &v3I, &vn);
            Triangle t;
            Vec3 v1 = mesh->vertices[v1I - 1];
            Vec3 v2 = mesh->vertices[v2I - 1];
            Vec3 v3 = mesh->vertices[v3I - 1];
            Vec3 e1; vec3_subtract(&v2, &v1, &e1);
            Vec3 e2; vec3_subtract(&v3, &v1, &e2);
            vec3_cross(&e1, &e2, &t.N);
            t.d = -vec3_dot(&t.N, &v1);
            vec3_cross(&e2, &t.N, &t.N1);
            float mag = vec3_magnitude(&t.N);
            mag = 1/(mag*mag);
            vec3_scale(&t.N1, mag, &t.N1);
            t.d1 = -vec3_dot(&t.N1, &v1);
            vec3_cross(&t.N, &e1, &t.N2);
            vec3_scale(&t.N2, mag, &t.N2);
            t.d2 = -vec3_dot(&t.N2, &v1);
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
    float det = vec3_dot(&ray->direction, &triangle->N);
    float t_ = -(vec3_dot(&ray->origin, &triangle->N) + triangle->d);
    Vec3 det_O; vec3_scale(&ray->origin, det, &det_O);
    Vec3 t_D; vec3_scale(&ray->direction, t_, &t_D);
    Vec3 P_; vec3_add(&det_O, &t_D, &P_);
    float u_ = (vec3_dot(&P_, &triangle->N1) + det*triangle->d1);
    float v_ = (vec3_dot(&P_, &triangle->N2) + det*triangle->d2);
    float temp_det = det - u_ - v_;
    int pdet0 = *(int*)&temp_det;
    int pdetu = *(int*)&u_;
    int pdetv = *(int*)&v_;
    pdet0 = pdet0 ^ pdetu;
    pdet0 = pdet0 | (pdetu ^ pdetv);
    if (pdet0 & 0x80000000)
        return 0;
    float det_inv = 1/det;
    float t = t_*det_inv;
    float u = u_*det_inv;
    float v = v_*det_inv;
    if (t >= epsilon) {
        return 1;
    }
    return 0;
}

float reflect(Ray *ray, Triangle *triangle, Vec3 *out){
    Vec3 normalized_tria; vec3_normalize(&triangle->N, &normalized_tria);
    float dot_prod = vec3_dot(&ray->direction, &triangle->N);
    vec3_scale(&triangle->N, -2*dot_prod, out);
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
