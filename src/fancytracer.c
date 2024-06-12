#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // Include the OpenMP header
#include "spatial.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const float FOCAL_LENGTH = 2.0f;
const int WIDTH = 800;
const int HEIGHT = 400;
const char *FILENAME = "output.png";
const char *OBJFILE = "baseScene.obj";

void render_scene(const char *filename, const int width, const int height, const char *objfile) {
    // Measure total execution time
    double total_start = omp_get_wtime();

    // Load mesh
    Mesh mesh = {0};
    read_obj_file(objfile, &mesh);

    Scene mainScene;
    buildScene(mesh.triangles, mesh.triangle_count, &mainScene, 3);
    test(&mainScene);

    Vec3 cam_pos = {0, 0, 5};
    Vec3 cam_rot = {0, 0, 0};
    Camera cam = {cam_pos, cam_rot, width, height, FOCAL_LENGTH};

    unsigned char *image = (unsigned char *)malloc(width * height * 3);
    if (!image) {
        printf("Error: Unable to allocate memory for image.\n");
        return;
    }

    // Measure time spent in ray_intersects_mesh and count the number of tests
    double ray_intersect_time = 0.0;
    int num_threads = 0;

    // Start parallel region
    #pragma omp parallel
    {
        double thread_ray_intersect_time = 0.0;
        long long thread_num_tests = 0;
        Ray cam_ray;
        cam_ray.origin = cam.position;

        #pragma omp for collapse(1) schedule(dynamic, 2) reduction(+:ray_intersect_time)
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                screen2CameraDir(&cam, x, y, &cam_ray.direction);
                int color = 0;
                Vec3 intersect_point;
                double ray_intersect_start = omp_get_wtime();
                int tria_ind = get_triangle_intersect(&cam_ray, &mesh, &intersect_point);
                if (tria_ind != -1) {
                    Vec3 out_reflect;
                    //color = (tria_ind+1)%255;
                    // color = (int)40*intersect_point.x;
                    color = (int)255 * reflect(&cam_ray, &intersect_point, &mesh.triangles[tria_ind], &out_reflect);
                }
                double ray_intersect_end = omp_get_wtime();
                thread_ray_intersect_time += (ray_intersect_end - ray_intersect_start);
                thread_num_tests++;
                image[(y * width + x) * 3] = (unsigned char)(color);                  // Blue
                image[(y * width + x) * 3 + 1] = (unsigned char)(color);              // Green
                image[(y * width + x) * 3 + 2] = (unsigned char)(color);              // Red
            }
        }

        // Combine the times and counts from all threads
        #pragma omp atomic
        ray_intersect_time += thread_ray_intersect_time;

        #pragma omp single
        {
            num_threads = omp_get_num_threads();
        }
    } // End parallel region

    if (!stbi_write_png(filename, width, height, 3, image, width * 3)) {
        printf("Error: Unable to write image to file %s.\n", filename);
        free(image);
        return;
    }
    freeScene(&mainScene);
    free(image);

    double total_end = omp_get_wtime();
    double total_time = total_end - total_start;
    double average_intersect_time = ray_intersect_time / num_threads;

    printf("Total execution time: %f seconds\n", total_time);
    printf("Average time per intersection test: %f seconds\n", average_intersect_time);
}

int main() {
    render_scene(FILENAME, WIDTH, HEIGHT, OBJFILE);
    printf("Image created successfully: %s\n", FILENAME);
    return 0;
}
