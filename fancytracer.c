#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void write_png_file(char *filename, int width, int height) {
    unsigned char *image = (unsigned char *)malloc(width * height * 3);
    if (!image) {
        fprintf("Error: Unable to allocate memory for image.\n");
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            image[(y * width + x) * 3] = (unsigned char)((y + x) * 255 / (height + width)); // Blue
            image[(y * width + x) * 3 + 1] = (unsigned char)(x * 255 / width);               // Green
            image[(y * width + x) * 3 + 2] = (unsigned char)(y * 255 / height);              // Red
        }
    }

    if (!stbi_write_png(filename, width, height, 3, image, width * 3)) {
        fprintf("Error: Unable to write image to file %s.\n", filename);
        free(image);
        return;
    }

    free(image);
}

int main() {
    char *filename = "output.png";
    int width = 800;
    int height = 600;
    write_png_file(filename, width, height);
    printf("Image created successfully: %s\n", filename);
    return 0;
}
