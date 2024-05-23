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

int ray_intersects_triangle(Vec3 ray_origin, Vec3 ray_vector, const Triangle *triangle, Vec3 *out_intersection_point) 
{
    
}

int ray_intersects_mesh(Vec3 ray_origin, Vec3 ray_vector, const Mesh *mesh, Vec3 *out_intersection_point){
    for (size_t i = 0; i < mesh->triangle_count; i++)
    {
        if (ray_intersects_triangle(ray_origin, ray_vector, &mesh->triangles[i], out_intersection_point)){
            return 1;
        }
    }
    return 0;
}

Vec3 screen2CameraDir(Camera cam, int screenPos_x, int screenPos_y){
    Vec3 cam_coor = {
        (float)screenPos_x/(float)cam.height,
        (float)screenPos_y/(float)cam.height,
        0
    };
    Vec3 center_shift = {
        -((float)cam.width/(float)cam.height)*0.5, -0.5, -cam.focal_length
    };
    Vec3 res = vec3_add(cam_coor, center_shift);
    return res;
}