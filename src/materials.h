#ifndef MATERIALS_H
#define MATERIALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "linalg.h"

// Struct to represent a texture
typedef struct {
    Vec3 *pixels;    // Array of pixels
    int width;       // Width of the texture
    int height;      // Height of the texture
} Texture;

typedef struct {
    Vec3 value;
    Texture tex;
    int uses_texture;
} Vec3OrTexture;

typedef struct {
    char name[64];  // Material name
    Vec3OrTexture color;           // Base color
    Vec3OrTexture metallic;        // Metallic
    Vec3OrTexture emissive;        // Emissive
    Vec3OrTexture specular;        // Specular
    Vec3OrTexture specular_roughness; // Specular roughness
    Vec3OrTexture specular_color;  // Specular color
} Material;

typedef struct {
    Material *mats;
    int material_count;
} Materials;

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

// Function to free a loaded texture
void free_texture(Texture *texture) {
    if (texture->pixels) {
        free(texture->pixels);
        texture->pixels = NULL;
        texture->width = 0;
        texture->height = 0;
    }
}

Materials load_materials(const char* mtl_filename) {
    FILE* file = fopen(mtl_filename, "r");
    Materials materials;
    materials.material_count = 0;
    int material_capacity = 10;
    materials.mats = malloc(material_capacity * sizeof(Material));

    char line[256];
    Material* current_material = NULL;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "newmtl ", 7) == 0) {
            if (materials.material_count >= material_capacity) {
                material_capacity *= 2;
                materials.mats = realloc(materials.mats, material_capacity * sizeof(Material));
            }
            current_material = &materials.mats[materials.material_count++];
            memset(current_material, 0, sizeof(Material));  // Set all to 0/NULL
            sscanf(line, "newmtl %63s", current_material->name);
        } else if (current_material) {
            // Base color
            if (strncmp(line, "Kd ", 3) == 0) {
                sscanf(line, "Kd %f %f %f", &current_material->color.value.x, 
                       &current_material->color.value.y, &current_material->color.value.z);
            } else if (strncmp(line, "map_Kd ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Kd %255s", texture_filename);
                current_material->color.tex = load_texture(texture_filename);
                current_material->color.uses_texture = 1;
            }
            // Metallic
            else if (strncmp(line, "Pm ", 3) == 0) {
                sscanf(line, "Pm %f", &current_material->metallic.value.x);
                current_material->metallic.value.y = current_material->metallic.value.x;
                current_material->metallic.value.z = current_material->metallic.value.x;
            } else if (strncmp(line, "map_Pm ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Pm %255s", texture_filename);
                current_material->metallic.tex = load_texture(texture_filename);
                current_material->metallic.uses_texture = 1;
            }
            // Emissive
            else if (strncmp(line, "Ke ", 3) == 0) {
                sscanf(line, "Ke %f %f %f", &current_material->emissive.value.x,
                       &current_material->emissive.value.y, &current_material->emissive.value.z);
            } else if (strncmp(line, "map_Ke ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Ke %255s", texture_filename);
                current_material->emissive.tex = load_texture(texture_filename);
                current_material->emissive.uses_texture = 1;
            }
            // Specular
            else if (strncmp(line, "Ks ", 3) == 0) {
                sscanf(line, "Ks %f %f %f", &current_material->specular.value.x,
                       &current_material->specular.value.y, &current_material->specular.value.z);
            } else if (strncmp(line, "map_Ks ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Ks %255s", texture_filename);
                current_material->specular.tex = load_texture(texture_filename);
                current_material->specular.uses_texture = 1;
            }
            // Specular roughness
            else if (strncmp(line, "Ns ", 3) == 0) {
                float ns;
                sscanf(line, "Ns %f", &ns);
                float roughness = 1.0f - (ns / 1000.0f); // Convert Ns to roughness
                current_material->specular_roughness.value.x = roughness;
                current_material->specular_roughness.value.y = roughness;
                current_material->specular_roughness.value.z = roughness;
            } else if (strncmp(line, "map_Ns ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Ns %255s", texture_filename);
                current_material->specular_roughness.tex = load_texture(texture_filename);
                current_material->specular_roughness.uses_texture = 1;
            }
            // Specular color (reusing Ks for consistency with earlier code)
            else if (strncmp(line, "Ks ", 3) == 0) {
                sscanf(line, "Ks %f %f %f", &current_material->specular_color.value.x, 
                       &current_material->specular_color.value.y, &current_material->specular_color.value.z);
            } else if (strncmp(line, "map_Ks ", 7) == 0) {
                char texture_filename[256];
                sscanf(line, "map_Ks %255s", texture_filename);
                current_material->specular_color.tex = load_texture(texture_filename);
                current_material->specular_color.uses_texture = 1;
            }
        }
    }
    fclose(file);
    return materials;
}

void free_vec3_or_texture(Vec3OrTexture *vot) {
    if (vot->uses_texture) {
        free_texture(&vot->tex);
    }
    vot->uses_texture = 0;
}

void free_material(Material *material) {
    if (material == NULL) {
        return;
    }
    free_vec3_or_texture(&material->color);
    free_vec3_or_texture(&material->metallic);
    free_vec3_or_texture(&material->emissive);
    free_vec3_or_texture(&material->specular);
    free_vec3_or_texture(&material->specular_roughness);
    free_vec3_or_texture(&material->specular_color);
}


void free_materials(Materials mats){
    for (int i = 0; i < mats.material_count; i++)
    {
        free_material(&mats.mats[i]);
    }
    free(mats.mats);
}

#endif // MATERIALS_H
