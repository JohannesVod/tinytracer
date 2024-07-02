#ifndef MATERIALS_H
#define MATERIALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "mesh.h"

// Struct to represent a texture
typedef struct {
    Vec3 *pixels;    // Array of pixels
    int width;       // Width of the texture
    int height;      // Height of the texture
} Texture;

typedef struct {
    char name[64];  // Material name

    // Base color
    union {
        Vec3 color;
        Texture *color_texture;
    };
    int has_color_texture;

    // Metallic
    union {
        float metallic;
        Texture *metallic_texture;
    };
    int has_metallic_texture;

    // Emissive
    union {
        float emissive;
        Texture *emissive_texture;
    };
    int has_emissive_texture;

    // Specular
    union {
        float specular;
        Texture *specular_texture;
    };
    int has_specular_texture;

    // Specular roughness
    union {
        float specular_roughness;
        Texture *specular_roughness_texture;
    };
    int has_specular_roughness_texture;

    // Specular color
    union {
        Vec3 specular_color;
        Texture *specular_color_texture;
    };
    int has_specular_color_texture;

} Material;

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
    texture.pixels = (Vec3 *)malloc(texture.width * texture.height * sizeof(Vec3));
    if (!texture.pixels) {
        fprintf(stderr, "Error: Memory allocation failed for texture pixels\n");
        stbi_image_free(data);
        exit(EXIT_FAILURE);
    }

    // Copy data into pixels
    for (int i = 0; i < texture.width * texture.height; ++i) {
        texture.pixels[i].x = (float)data[i * 4]/255;
        texture.pixels[i].y = (float)data[i * 4 + 1]/255;
        texture.pixels[i].z = (float)data[i * 4 + 2]/255;
    }

    // Free original image data
    stbi_image_free(data);
    return texture;
}

/* Gets pixel from texture coordinate */
Vec3 GetPixel(Vec2 *vc, Texture *tex){
    int x = ((int) (vc->x*tex->width))%tex->width;
    int y = (tex->height - (int) (vc->y*tex->height))%tex->height;
    return tex->pixels[y*tex->width+x];
}

/* Gets pixel from barycentric coordinates */
Vec3 GetPixelFromTria(Texture *tex, Triangle *t, Vec3 *barycentric){
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

void GetTriangleNormal(Triangle *triangle, Vec3 *barycentric, Vec3 *out){
    float u = barycentric->y;
    float v = barycentric->z;
    float w = 1 - u - v;
    Vec3 vn1, vn2, vn3;
    vec3_scale(&triangle->vn1, w, &vn1);
    vec3_scale(&triangle->vn2, u, &vn2);
    vec3_scale(&triangle->vn3, v, &vn3);
    vec3_add(&vn1, &vn2, out);
    vec3_add(out, &vn3, out);
    vec3_normalize(out, out); // TODO: maybe not needed
}

int reflect(Ray *ray, Triangle *triangle, Vec3 *tria_normal, Vec3 *out){
    // Reflect the ray direction along the normal
    float dot_prod = vec3_dot(&ray->direction, tria_normal);
    vec3_scale(tria_normal, -2 * dot_prod, out);
    vec3_add(&ray->direction, out, out);
    return 1;
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

Material* load_material(const char* mtl_filename) {
    FILE* file = fopen(mtl_filename, "r");
    if (!file) {
        perror("Error opening material file");
        return NULL;
    }

    Material* materials = NULL;
    int material_count = 0;
    int material_capacity = 10;
    materials = malloc(material_capacity * sizeof(Material));

    char line[256];
    Material* current_material = NULL;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "newmtl ", 7) == 0) {
            if (material_count >= material_capacity) {
                material_capacity *= 2;
                materials = realloc(materials, material_capacity * sizeof(Material));
            }
            current_material = &materials[material_count++];
            memset(current_material, 0, sizeof(Material));  // Set all to 0/NULL
            sscanf(line, "newmtl %63s", current_material->name);
        } else if (current_material) {
            // Base color
            if (strncmp(line, "Kd ", 3) == 0) {
                sscanf(line, "Kd %f %f %f", &current_material->color.x, 
                       &current_material->color.y, &current_material->color.z);
            } else if (strncmp(line, "map_Kd ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Kd %255s", texture_filename);
                current_material->color_texture = load_texture(texture_filename);
                current_material->has_color_texture = 1;
            }
            // Metallic
            else if (strncmp(line, "Pm ", 3) == 0) {
                sscanf(line, "Pm %f", &current_material->metallic);
            } else if (strncmp(line, "map_Pm ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Pm %255s", texture_filename);
                current_material->metallic_texture = load_texture(texture_filename);
                current_material->has_metallic_texture = 1;
            }
            // Emissive
            else if (strncmp(line, "Ke ", 3) == 0) {
                sscanf(line, "Ke %f", &current_material->emissive);
            } else if (strncmp(line, "map_Ke ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Ke %255s", texture_filename);
                current_material->emissive_texture = load_texture(texture_filename);
                current_material->has_emissive_texture = 1;
            }
            // Specular
            else if (strncmp(line, "Ks ", 3) == 0) {
                sscanf(line, "Ks %f", &current_material->specular);
            } else if (strncmp(line, "map_Ks ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Ks %255s", texture_filename);
                current_material->specular_texture = load_texture(texture_filename);
                current_material->has_specular_texture = 1;
            }
            // Specular roughness
            else if (strncmp(line, "Ns ", 3) == 0) {
                float ns;
                sscanf(line, "Ns %f", &ns);
                current_material->specular_roughness = 1.0f - (ns / 1000.0f); // Convert Ns to roughness
            } else if (strncmp(line, "map_Ns ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Ns %255s", texture_filename);
                current_material->specular_roughness_texture = load_texture(texture_filename);
                current_material->has_specular_roughness_texture = 1;
            }
            // Specular color
            else if (strncmp(line, "Ks ", 3) == 0) {
                sscanf(line, "Ks %f %f %f", &current_material->specular_color.x, 
                       &current_material->specular_color.y, &current_material->specular_color.z);
            } else if (strncmp(line, "map_Ks ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Ks %255s", texture_filename);
                current_material->specular_color_texture = load_texture(texture_filename);
                current_material->has_specular_color_texture = 1;
            }
        }
    }

    fclose(file);
    return materials;
}

#endif // MATERIALS_H
