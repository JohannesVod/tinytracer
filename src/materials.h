#ifndef MATERIALS_H
#define MATERIALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Struct to represent a single pixel
typedef struct {
    unsigned char r, g, b, a;  // Red, Green, Blue, Alpha values
} Pixel;

// Struct to represent a texture
typedef struct {
    Pixel *pixels;   // Array of pixels
    int width;       // Width of the texture
    int height;      // Height of the texture
} Texture;

// Function to load a texture from a file
Texture load_texture(const char *filename) {
    Texture texture;
    int channels;
    unsigned char *data = stbi_load(filename, &texture.width, &texture.height, &channels, 4); // Force 4 channels for RGBA
    if (!data) {
        fprintf(stderr, "Error: Unable to load texture file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // Allocate memory for pixels
    texture.pixels = (Pixel *)malloc(texture.width * texture.height * sizeof(Pixel));
    if (!texture.pixels) {
        fprintf(stderr, "Error: Memory allocation failed for texture pixels\n");
        stbi_image_free(data);
        exit(EXIT_FAILURE);
    }

    // Copy data into pixels
    for (int i = 0; i < texture.width * texture.height; ++i) {
        texture.pixels[i].r = data[i * 4];
        texture.pixels[i].g = data[i * 4 + 1];
        texture.pixels[i].b = data[i * 4 + 2];
        texture.pixels[i].a = data[i * 4 + 3];
    }

    // Free original image data
    stbi_image_free(data);
    return texture;
}

// Function to free a loaded texture
void free_texture(Texture *texture) {
    if (texture->pixels) {
        free(texture->pixels);
        texture->pixels = NULL;
        texture->width = 0;
        texture->height = 0;
    }
}

#endif // MATERIALS_H
