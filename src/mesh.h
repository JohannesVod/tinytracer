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
    Vec2 vt1, vt2, vt3; // textures coordinates
    int material;
} Triangle;

typedef struct {
    Vec3 p;
    float rad;
} Sphere;

typedef struct {
    Triangle *triangles;
    int count;
} Triangles;

typedef struct {
    Vec3 position, rotation;
    int width;
    int height;
    float focal_length;
} Camera;

/* reads object file and saves triangles */
void read_obj_file(const char *filename, Triangles *mesh) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int vertex_capacity = 10;
    int normal_capacity = 10;
    int triangle_capacity = 10;
    int texcoors_capacity = 10;
    Vec3 *vertices = malloc(vertex_capacity * sizeof(Vec3));
    Vec3 *normals = malloc(normal_capacity * sizeof(Vec3));
    Vec2 *texCoors = malloc(texcoors_capacity * sizeof(Vec2));

    int normal_count = 0;
    int vertex_count = 0;
    int texcoors_count = 0;
    mesh->triangles = malloc(triangle_capacity * sizeof(Triangle));
    mesh->count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            if (vertex_count >= vertex_capacity) {
                vertex_capacity *= 2;
                vertices = realloc(vertices, vertex_capacity * sizeof(Vec3));
            }
            Vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            vertices[vertex_count++] = v;
        } else if (strncmp(line, "vt ", 3) == 0) {
            if (texcoors_count >= texcoors_capacity) {
                texcoors_capacity *= 2;
                texCoors = realloc(texCoors, texcoors_capacity * sizeof(Vec2));
            }
            Vec2 vt;
            sscanf(line, "vt %f %f", &vt.x, &vt.y);
            texCoors[texcoors_count++] = vt;
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (normal_count >= normal_capacity) {
                normal_capacity *= 2;
                normals = realloc(normals, normal_capacity * sizeof(Vec3));
            }
            Vec3 n;
            sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
            normals[normal_count++] = n;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (mesh->count >= triangle_capacity) {
                triangle_capacity *= 2;
                mesh->triangles = realloc(mesh->triangles, triangle_capacity * sizeof(Triangle));
            }
            int v1, v2, v3;
            int vt1, vt2, vt3;
            int vn1, vn2, vn3;
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3);

            Triangle t;
            vec3_copy(&vertices[v1 - 1], &t.v1);
            vec3_copy(&vertices[v2 - 1], &t.v2);
            vec3_copy(&vertices[v3 - 1], &t.v3);

            vec2_copy(&texCoors[vt1 - 1], &t.vt1);
            vec2_copy(&texCoors[vt2 - 1], &t.vt2);
            vec2_copy(&texCoors[vt3 - 1], &t.vt3);

            vec3_copy(&normals[vn1 - 1], &t.vn1);
            vec3_copy(&normals[vn2 - 1], &t.vn2);
            vec3_copy(&normals[vn3 - 1], &t.vn3);

            mesh->triangles[mesh->count++] = t;
        }
    }
    free(vertices);
    free(normals);
    free(texCoors);
    fclose(file);
}

void free_triangles(Triangles *mesh) {
    free(mesh->triangles);
}

/* checks if ray intersects triangle and stores barycentric coordinates in out*/
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
    if (u < 0 || u > 1.0) {
        return 0;
    }
    vec3_cross(&b, &e1, &b_cross_e1);
    float v = inv_det * vec3_dot(&ray->direction, &b_cross_e1); // v
    if (v < 0 || v + u > 1.0) {
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
/* converts 2d pixel to camera ray */
int screen2CameraDir(Camera *cam, int screenPos_x, int screenPos_y, Vec3 *result) {
    float x = (float) screenPos_x + randFloat();
    float y = (float) screenPos_y + randFloat();
    Vec3 cam_coor = {
        x / (float)cam->height,
        y / (float)cam->height,
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