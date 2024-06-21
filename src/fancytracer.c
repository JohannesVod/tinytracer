#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // Include the OpenMP header
#include "materials.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const float FOCAL_LENGTH = 2.5f;
const int WIDTH = 800;
const int HEIGHT = 400; 
const int SAMPLES = 1000;
const int gridcells = 30;
const char *FILENAME = "output.png";
const char *OBJFILE = "baseScene.obj";

void storeImage(unsigned char *image, float *image_buff, int curr_samples) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float x_img = image_buff[(y * WIDTH + x) * 3]/curr_samples;
            float y_img = image_buff[(y * WIDTH + x) * 3+1]/curr_samples;
            float z_img = image_buff[(y * WIDTH + x) * 3+2]/curr_samples;
            if (x_img > 255){x_img = 255;}
            if (y_img > 255){y_img = 255;}
            if (z_img > 255){z_img = 255;}
            image[(y * WIDTH + x) * 3] = (unsigned char)x_img;
            image[(y * WIDTH + x) * 3 + 1] = (unsigned char)y_img;
            image[(y * WIDTH + x) * 3 + 2] = (unsigned char)z_img;
        }
    }
    if (!stbi_write_png(FILENAME, WIDTH, HEIGHT, 3, image, WIDTH * 3)) {
        printf("Error: Unable to write image to file %s.\n", FILENAME);
    }
}

void render_scene() {
    // Measure total execution time
    double preprocess_start = omp_get_wtime();
    // Load mesh
    Triangles triangles;
    read_obj_file(OBJFILE, &triangles);
    Texture tex = load_texture("testTex.png");

    Vec3 cam_pos = {0, 0, 5}; 
    Vec3 cam_rot = {0, 0, 0};
    Camera cam = {cam_pos, cam_rot, WIDTH, HEIGHT, FOCAL_LENGTH};

    Scene mainScene;
    buildScene(&cam, &triangles, &mainScene, gridcells);
    double preprocess_end = omp_get_wtime();
    double prepocess_time = preprocess_end - preprocess_start;
    printf("Preprocessed in: %f seconds\n", prepocess_time);

    double total_start = omp_get_wtime();
    unsigned char *image = (unsigned char *)malloc(WIDTH * HEIGHT * 3);
    float *image_buff = (float *)malloc(WIDTH * HEIGHT * sizeof(float) * 3);
    if (!image || !image_buff) {
        printf("Error: Unable to allocate memory for image.\n");
        return;
    }
    // Start parallel region
    #pragma omp parallel
    {
        Ray cam_ray;
        cam_ray.origin = cam.position;
        for (int sampl = 0; sampl < SAMPLES; sampl++)
        {
            #pragma omp for collapse(1) schedule(dynamic, 2)
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    screen2CameraDir(&cam, x, y, &cam_ray.direction);
                    Vec3 pix = trace(&mainScene, &cam_ray, 4);
                    int this_y = HEIGHT - y - 1;
                    image_buff[(this_y * WIDTH + x) * 3] += pix.x*255;            // Red
                    image_buff[(this_y * WIDTH + x) * 3 + 1] += pix.y*255;        // Green
                    image_buff[(this_y * WIDTH + x) * 3 + 2] += pix.z*255;        // Blue
                }
            }
            #pragma omp single
            {
                storeImage(image, image_buff, sampl+1);
                printf("sample %d/%d\n", sampl+1, SAMPLES);
            }
        }
       
    } // End parallel region

    freeScene(&mainScene);
    free(image);
    free(image_buff);

    double total_end = omp_get_wtime();
    double total_time = total_end - total_start;

    printf("Total execution time: %f seconds\n", total_time);
}

int main() {
    srand(time(NULL));
    render_scene();
    printf("Image created successfully: %s\n", FILENAME);
    return 0;
}
