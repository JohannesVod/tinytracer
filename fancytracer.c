#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>


typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    Vec3 v1, v2, v3;
} Triangle;

typedef struct {
    Vec3 *vertices;
    size_t vertex_count;
    Triangle *triangles;
    size_t triangle_count;
} Mesh;

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
        return 0;    // This ray is parallel to this triangle.

    float inv_det = 1.0f / det;
    Vec3 s = vec3_subtract(ray_origin, triangle->v1);
    float u = inv_det * vec3_dot(s, ray_cross_e2);

    if (u < 0.0f || u > 1.0f)
        return 0;

    Vec3 s_cross_e1 = vec3_cross(s, edge1);
    float v = inv_det * vec3_dot(ray_vector, s_cross_e1);

    if (v < 0.0f || u + v > 1.0f)
        return 0;

    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = inv_det * vec3_dot(edge2, s_cross_e1);

    if (t > epsilon) // ray intersection
    {
        *out_intersection_point = vec3_add(ray_origin, vec3_scale(ray_vector, t));
        return 1;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return 0;
}

Vec3 screen2Camera(Vec3 camera_pos, ){
    
}

int main() {
    Mesh mesh = {0};
    read_obj_file("pyramid.obj", &mesh);

    // Example usage: Print triangles
    for (size_t i = 0; i < mesh.triangle_count; ++i) {
        Triangle t = mesh.triangles[i];
        printf("Triangle %zu:\n", i + 1);
        printf("  v1: (%f, %f, %f)\n", t.v1.x, t.v1.y, t.v1.z);
        printf("  v2: (%f, %f, %f)\n", t.v2.x, t.v2.y, t.v2.z);
        printf("  v3: (%f, %f, %f)\n", t.v3.x, t.v3.y, t.v3.z);
    }

    free_mesh(&mesh);
    return 0;
}