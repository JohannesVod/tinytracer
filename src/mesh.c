// mesh.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "mesh.h"
#include "linalg.h"

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

int ray_intersects_triangle(Vec3 ray_origin, 
                             Vec3 ray_vector, 
                             const Triangle *triangle,
                             Vec3 *out_intersection_point) 
{
    const float epsilon = FLT_EPSILON;

    Vec3 edge1 = vec3_subtract(triangle->v2, triangle->v1);
    Vec3 edge2 = vec3_subtract(triangle->v3, triangle->v1);
    Vec3 ray_cross_e2 = vec3_cross(ray_vector, edge2);
    float det = vec3_dot(edge1, ray_cross_e2);

    if (det > -epsilon && det < epsilon)
        return 0;

    float inv_det = 1.0f / det;
    Vec3 s = vec3_subtract(ray_origin, triangle->v1);
    float u = inv_det * vec3_dot(s, ray_cross_e2);

    if (u < 0.0f || u > 1.0f)
        return 0;

    Vec3 s_cross_e1 = vec3_cross(s, edge1);
    float v = inv_det * vec3_dot(ray_vector, s_cross_e1);

    if (v < 0.0f || u + v > 1.0f)
        return 0;

    float t = inv_det * vec3_dot(edge2, s_cross_e1);

    if (t > epsilon)
    {
        *out_intersection_point = vec3_add(ray_origin, vec3_scale(ray_vector, t));
        return 1;
    }
    else
        return 0;
}

Vec3 screen2CameraDir(Camera cam, int screenPos_x, int screenPos_y){
    Vec3 cam_coor = {
        (float)screenPos_x/(float)cam.height,
        (float)screenPos_y/(float)cam.height,
        0
    };
    return cam_coor;
    // Vec3 center_shift = {
    //     -0.5, -((float)cam.width/(float)cam.height)*0.5, 0
    // };
    // Vec3 shifted = vec3_add(cam_coor, center_shift);
}