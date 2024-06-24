#ifndef MATERIALS_H
#define MATERIALS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spatial.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

int reflect(Ray *ray, Vec3 *barycentric, Triangle *triangle, Vec3 *out){
    // Interpolating the normal using barycentric coordinates
    Vec3 normal;
    GetTriangleNormal(triangle, barycentric, &normal);

    // Reflect the ray direction along the normal
    float dot_prod = vec3_dot(&ray->direction, &normal);
    vec3_scale(&normal, -2 * dot_prod, out);
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

Vec3 trace(Scene *scene, Ray *cam_ray, int bounces, Texture *tex){
    Ray curr_ray;
    vec3_copy(&cam_ray->origin, &curr_ray.origin);
    vec3_copy(&cam_ray->direction, &curr_ray.direction);
    Vec3 res;
    res.x = 1; res.y = 1; res.z = 1;
    for (int bounce = 0; bounce < bounces; bounce++)
    {
        Vec3 barycentric;
        int tria_ind = castRay(&curr_ray, scene, &barycentric);
        if (tria_ind != -1) { // intersection found!
            Triangle *this_tria = &scene->triangles->triangles[tria_ind];
            Vec3 tria_normal;
            GetTriangleNormal(this_tria, &barycentric, &tria_normal);
            // tria_normal.x = (tria_normal.x + 1)/2;
            // tria_normal.y = (tria_normal.y + 1)/2;
            // tria_normal.z = (tria_normal.z + 1)/2;
            // return tria_normal;
            // reflection direction:
            Vec3 out_reflect;
            reflect(&curr_ray, &barycentric, this_tria, &out_reflect);
            // diffuse direction:
            Material *this_mat = &scene->mats[this_tria->material];
            vec3_mul(&res, &this_mat->color, &res);
            if (this_mat->emissive > 0){
                vec3_scale(&res, this_mat->emissive, &res);
                return res;
            }
            Vec3 diffuse = rand_lambertian(&tria_normal);
            if (this_mat->metallic > 0){
                vec3_copy(&out_reflect, &diffuse);
            }
            // calc new ray
            Vec3 dir_scaled; vec3_copy(&curr_ray.direction, &dir_scaled);
            vec3_scale(&dir_scaled, barycentric.x, &dir_scaled);
            vec3_add(&curr_ray.origin, &dir_scaled, &curr_ray.origin);
            vec3_copy(&diffuse, &curr_ray.direction);
        }
        else{
            break;
        }
    }
    vec3_scale(&res, 0.0, &res); // value denotes environment lighting 
    return res;
}

#endif // MATERIALS_H
