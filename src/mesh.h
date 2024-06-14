#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "linalg.h"

typedef struct {
    Vec3 v1, v2, v3;
    Vec3 vn1, vn2, vn3; // maybe remove later to increase cache locality
    int material;
} Triangle;

typedef struct {
    Vec3 p;
    float rad;
} Sphere;

typedef struct {
    Triangle *triangles;
    int triangle_count;
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
    int vertex_capacity = 10;
    int normal_capacity = 10;
    int triangle_capacity = 10;
    Vec3 *vertices = malloc(vertex_capacity * sizeof(Vec3));
    Vec3 *normals = malloc(normal_capacity * sizeof(Vec3));
    int normal_count = 0;
    int vertex_count = 0;
    mesh->triangles = malloc(triangle_capacity * sizeof(Triangle));
    mesh->triangle_count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            if (vertex_count >= vertex_capacity) {
                vertex_capacity *= 2;
                vertices = realloc(vertices, vertex_capacity * sizeof(Vec3));
            }
            Vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            vertices[vertex_count++] = v;
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (normal_count >= normal_capacity) {
                normal_capacity *= 2;
                normals = realloc(normals, normal_capacity * sizeof(Vec3));
            }
            Vec3 n;
            sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
            normals[normal_count++] = n;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (mesh->triangle_count >= triangle_capacity) {
                triangle_capacity *= 2;
                mesh->triangles = realloc(mesh->triangles, triangle_capacity * sizeof(Triangle));
            }
            int v1, v2, v3;
            int vn;
            sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%d", &v1, &v2, &v3, &vn);
            Triangle t;
            t.v1 = vertices[v1 - 1];
            t.v2 = vertices[v2 - 1];
            t.v3 = vertices[v3 - 1];

            Vec3 e1, e2;
            vec3_subtract(&t.v2, &t.v1, &e1);
            vec3_subtract(&t.v3, &t.v1, &e2);
            Vec3 this_normals; vec3_cross(&e1, &e2, &this_normals);
            vec3_normalize(&this_normals, &this_normals);

            t.vn1 = normals[v1 - 1];
            t.vn2 = normals[v2 - 1];
            t.vn3 = normals[v3 - 1];
            // t.normal = normals[vn - 1];
            //calculate_normal(&t);
            mesh->triangles[mesh->triangle_count++] = t;
        }
    }
    free(vertices);
    free(normals);
    fclose(file);
}

void free_mesh(Mesh *mesh) {
    free(mesh->triangles);
}

int ray_intersects_triangle(Ray *ray, Triangle *triangle, Vec3 *out) {
    const float epsilon = 1e-6;
    const float epsilon2 = 1e-3;
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
    if (u < -epsilon2 || u > 1.0+epsilon2) {
        return 0;
    }
    vec3_cross(&b, &e1, &b_cross_e1);
    float v = inv_det * vec3_dot(&ray->direction, &b_cross_e1); // v
    if (v < -epsilon2 || v + u > 1.0+epsilon2) {
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

float reflect(Ray *ray, Vec3 *barycentric, Triangle *triangle, Vec3 *out){
    float u = barycentric->y;
    float v = barycentric->z;
    float w = 1 - u - v;
    Vec3 normal;
    Vec3 vn1, vn2, vn3; 
    vec3_scale(&triangle->vn1, w, &vn1);
    vec3_scale(&triangle->vn2, u, &vn2);
    vec3_scale(&triangle->vn3, v, &vn3);
    vec3_add(&vn1, &vn2, &normal);
    vec3_add(&normal, &vn3, &normal);
    vec3_normalize(&normal, &normal); // needed?
    float dot_prod = vec3_dot(&ray->direction, &normal);
    vec3_scale(&normal, -2*dot_prod, out);
    vec3_add(&ray->direction, out, out);
    return dot_prod;
}

int get_triangle_intersect(Ray *ray, Mesh *mesh, Vec3 *out){
    float min_t = 1e10;
    int tria_ind = -1;
    Vec3 out_temp;
    for (int i = 0; i < mesh->triangle_count; i++) {
        if (ray_intersects_triangle(ray, &mesh->triangles[i], &out_temp)) {
            if (out_temp.x < min_t){
                vec3_copy(&out_temp, out);
                min_t = out_temp.x;
                tria_ind = i;
            }
        }
    }
    return tria_ind;
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