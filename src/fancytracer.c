#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h> // Include the OpenMP header
#include "toneMapping.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const float FOCAL_LENGTH = 3.7f;
const int WIDTH = 1600;
const int HEIGHT = 800;
const float DOF = 0.018;
const float FSTOP = 4.7;
const int SAMPLES = 30000;
const int BOUNCES = 3;
const int gridcells = 150; // 150 for motorbike please
const char *FILENAME = "output.png";
const char *OBJFILE = "scene/baseScene.obj";
const char *MATFILENAME = "scene/baseScene.mtl";
const char *TEXTURESFOLDER = "scene/textures";

void storeImage(unsigned char *image, float *image_buff, int curr_samples) {
    float max_v = 0;
    for (int i = 0; i < HEIGHT*WIDTH*3; i++) {
        if (image_buff[i] > max_v){
            max_v = image_buff[i];
        }
    }
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Vec3 c;
            c.x = image_buff[(y * WIDTH + x) * 3]/curr_samples;
            c.y = image_buff[(y * WIDTH + x) * 3+1]/curr_samples;
            c.z = image_buff[(y * WIDTH + x) * 3+2]/curr_samples;
            c = reinhard_extended_luminance(c, max_v);
            if (c.x > 1){ c.x = 1; }
            if (c.y > 1){ c.y = 1; }
            if (c.z > 1){ c.z = 1; }

            image[(y * WIDTH + x) * 3] = (unsigned char)(c.x*255);
            image[(y * WIDTH + x) * 3 + 1] = (unsigned char)(c.y*255);
            image[(y * WIDTH + x) * 3 + 2] = (unsigned char)(c.z*255);
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
    Materials mats = load_materials(MATFILENAME);
    for (int i = 0; i < mats.material_count; i++)
    {
        Material m = mats.mats[i];
        print_material(&m);
    }
    
    Triangles triangles = read_obj_file(OBJFILE, &mats);
    Vec3 cam_pos = {0, 0, 5};
    Vec3 cam_rot = {0, 0, 0};
    Camera cam = {cam_pos, cam_rot, WIDTH, HEIGHT, FOCAL_LENGTH};

    Scene mainScene;
    buildScene(&cam, &triangles, &mainScene, gridcells, mats);
    double preprocess_end = omp_get_wtime();
    double prepocess_time = preprocess_end - preprocess_start;
    printf("Preprocessed in: %f seconds\n", prepocess_time);

    double total_start = omp_get_wtime();
    unsigned char *image = (unsigned char *)malloc(WIDTH * HEIGHT * 3);
    float *image_buff = (float *)malloc(WIDTH * HEIGHT * sizeof(float) * 3);
    for (size_t i = 0; i < WIDTH * HEIGHT * 3; i++)
    {
        image_buff[i] = 0;
    }
    
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
            #pragma omp for collapse(2) schedule(dynamic, 2)
            for (int y = 0; y < HEIGHT; y++) {
                for (int x = 0; x < WIDTH; x++) {
                    screen2CameraDir(&cam, DOF, FSTOP, x, y, &cam_ray);
                    Vec3 pix = trace(&mainScene, &cam_ray, BOUNCES);
                    int this_y = HEIGHT - y - 1;
                    image_buff[(this_y * WIDTH + x) * 3] += pix.x;            // Red
                    image_buff[(this_y * WIDTH + x) * 3 + 1] += pix.y;        // Green
                    image_buff[(this_y * WIDTH + x) * 3 + 2] += pix.z;        // Blue
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
