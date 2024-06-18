#ifndef MATERIALS_H
#define MATERIALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spatial.h"

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

/* Gets pixel from texture coordinate */
Pixel GetPixel(Vec2 *vc, Texture *tex){
    int x = ((int) (vc->x*tex->width))%tex->width;
    int y = (tex->height - (int) (vc->y*tex->height))%tex->height;
    return tex->pixels[y*tex->width+x];
}

/* Gets pixel from barycentric coordinates */
Pixel GetPixelFromTria(Texture *tex, Triangle *t, Vec3 *barycentric){
    Vec2 e1, e2;
    vec2_subtract(&t->vt2, &t->vt1, &e1);
    vec2_subtract(&t->vt3, &t->vt1, &e2);
    vec2_scale(&e1, barycentric->y, &e1);
    vec2_scale(&e2, barycentric->z, &e2);
    Vec2 coor;
    vec2_copy(&t->vt1, &coor);
    vec2_add(&coor, &e1, &coor);
    vec2_add(&coor, &e2, &coor);

    return GetPixel(&coor, tex);
}

/* calculates reflected ray along triangle normal */
Pixel reflect(Ray *ray, Vec3 *barycentric, Triangle *triangle, Vec3 *out, Texture *tex){
    float u = barycentric->y;
    float v = barycentric->z;
    float w = 1 - u - v;
    Vec3 normal;
    Vec3 vn1, vn2, vn3;
    vec3_scale(&triangle->vn1, w, &vn1);
    vec3_scale(&triangle->vn2, u, &vn2);
    vec3_scale(&triangle->vn3, v, &vn3);
    vec3_add(&vn1, &vn2, &normal);
    vec3_add(&normal, &vn3, &normal);
    vec3_normalize(&normal, &normal); // needed?
    float dot_prod = vec3_dot(&ray->direction, &normal);
    vec3_scale(&normal, -2*dot_prod, out);
    vec3_add(&ray->direction, out, out);
    out; 
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
