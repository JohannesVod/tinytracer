#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mesh.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void render_scene(char *filename, int width, int height, char *objfile) {
    // Measure total execution time
    clock_t total_start = clock();

    // load mesh
    Mesh mesh = {0};
    read_obj_file(objfile, &mesh);
    Vec3 cam_pos = {0, 0, 5};
    Vec3 cam_rot = {0, 0, 0};
    float focal_length = 1;
    Camera cam = {cam_pos, cam_rot, width, height, focal_length};

    // Example usage: Print triangles
    // for (size_t i = 0; i < mesh.triangle_count; ++i) {
    //     Triangle t = mesh.triangles[i];
    //     printf("Triangle %zu:\n", i + 1);
    //     printf("  v1: (%f, %f, %f)\n", t.v1.x, t.v1.y, t.v1.z);
    //     printf("  v2: (%f, %f, %f)\n", t.v2.x, t.v2.y, t.v2.z);
    //     printf("  v3: (%f, %f, %f)\n", t.v3.x, t.v3.y, t.v3.z);
    // }

    unsigned char *image = (unsigned char *)malloc(width * height * 3);
    if (!image) {
        printf("Error: Unable to allocate memory for image.\n");
        return;
    }

    Vec3 intersect_point = {0, 0, 0};

    // Measure time spent in ray_intersects_mesh
    clock_t ray_intersect_start, ray_intersect_end;
    double ray_intersect_time = 0.0;
    Ray cam_ray;
    cam_ray.origin = cam.position;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            screen2CameraDir(&cam, x, y, &cam_ray.direction);
            int color = 255;

            ray_intersect_start = clock();
            if (ray_intersects_mesh(&cam_ray, &mesh, &intersect_point)) {
                color = (int)255*reflectMesh(&cam_ray, &mesh, &intersect_point);
                printf("%f", reflectMesh(&cam_ray, &mesh, &intersect_point));
            }
            ray_intersect_end = clock();
            ray_intersect_time += (double)(ray_intersect_end - ray_intersect_start) / CLOCKS_PER_SEC;
            image[(y * width + x) * 3] = (unsigned char)(color);                  // Blue
            image[(y * width + x) * 3 + 1] = (unsigned char)(color);              // Green
            image[(y * width + x) * 3 + 2] = (unsigned char)(color);              // Red
        }
    }

    if (!stbi_write_png(filename, width, height, 3, image, width * 3)) {
        printf("Error: Unable to write image to file %s.\n", filename);
        free(image);
        return;
    }
    free_mesh(&mesh);
    free(image);

    clock_t total_end = clock();
    double total_time = (double)(total_end - total_start) / CLOCKS_PER_SEC;

    printf("Total execution time: %f seconds\n", total_time);
    printf("Time spent in ray_intersects_mesh: %f seconds\n", ray_intersect_time);
}


int main() {
    char *filename = "output.png";
    int width = 800;
    int height = 400;
    render_scene(filename, width, height, "baseScene.obj");
    printf("Image created successfully: %s\n", filename);
    return 0;
}