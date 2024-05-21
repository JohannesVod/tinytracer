// main.c
#include <stdio.h>
#include "mesh.h"

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
