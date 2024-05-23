#include <stdio.h>
#include "mesh.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void render_scene(char *filename, int width, int height, char *objfile) {
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
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vec3 cam_ray = screen2CameraDir(cam, x, y);
            int color = 255;
            if (ray_intersects_mesh(cam.position, cam_ray, &mesh, &intersect_point)){
                color = 0;
            }
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
}

int main() {
    char *filename = "output.png";
    int width = 800;
    int height = 400;
    render_scene(filename, width, height, "baseScene.obj");
    printf("Image created successfully: %s\n", filename);
    return 0;
}