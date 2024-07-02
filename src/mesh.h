#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "materials.h"

typedef struct {
    Vec3 v1, v2, v3;
    Vec3 vn1, vn2, vn3; // maybe remove later to increase cache locality
    Vec2 vt1, vt2, vt3; // textures coordinates
    Material *material;
} Triangle;

typedef struct {
    Vec3 p;
    float rad;
} Sphere;

typedef struct {
    Triangle *triangles;
    int count;
} Triangles;

typedef struct {
    Vec3 position, rotation;
    int width;
    int height;
    float focal_length;
} Camera;

Material* find_material_by_name(Materials *mats, const char *name) {
    for (int i = 0; i < mats->material_count; i++) {
        if (strcmp(mats->mats[i].name, name) == 0) {
            return &mats->mats[i];
        }
    }
    return NULL; // Material not found
}
Triangles read_obj_file(const char *filename, Materials *mats) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    int vertex_capacity = 10;
    int normal_capacity = 10;
    int triangle_capacity = 10;
    int texcoors_capacity = 10;

    Vec3 *vertices = malloc(vertex_capacity * sizeof(Vec3));
    Vec3 *normals = malloc(normal_capacity * sizeof(Vec3));
    Vec2 *texCoors = malloc(texcoors_capacity * sizeof(Vec2));

    int normal_count = 0;
    int vertex_count = 0;
    int texcoors_count = 0;

    Triangles mesh;
    mesh.triangles = malloc(triangle_capacity * sizeof(Triangle));
    mesh.count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            if (vertex_count >= vertex_capacity) {
                vertex_capacity *= 2;
                vertices = realloc(vertices, vertex_capacity * sizeof(Vec3));
            }
            Vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            vertices[vertex_count++] = v;
        } else if (strncmp(line, "vt ", 3) == 0) {
            if (texcoors_count >= texcoors_capacity) {
                texcoors_capacity *= 2;
                texCoors = realloc(texCoors, texcoors_capacity * sizeof(Vec2));
            }
            Vec2 vt;
            sscanf(line, "vt %f %f", &vt.x, &vt.y);
            texCoors[texcoors_count++] = vt;
        } else if (strncmp(line, "vn ", 3) == 0) {
            if (normal_count >= normal_capacity) {
                normal_capacity *= 2;
                normals = realloc(normals, normal_capacity * sizeof(Vec3));
            }
            Vec3 n;
            sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
            normals[normal_count++] = n;
        } else if (strncmp(line, "f ", 2) == 0) {
            if (mesh.count >= triangle_capacity) {
                triangle_capacity *= 2;
                mesh.triangles = realloc(mesh.triangles, triangle_capacity * sizeof(Triangle));
            }
            int v1, v2, v3;
            int vt1, vt2, vt3;
            int vn1, vn2, vn3;
            char material_name[64];
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %63s", 
                   &v1, &vt1, &vn1, &v2, &vt2, &vn2, &v3, &vt3, &vn3, material_name);
            
            Triangle t;
            vec3_copy(&vertices[v1 - 1], &t.v1);
            vec3_copy(&vertices[v2 - 1], &t.v2);
            vec3_copy(&vertices[v3 - 1], &t.v3);
            vec2_copy(&texCoors[vt1 - 1], &t.vt1);
            vec2_copy(&texCoors[vt2 - 1], &t.vt2);
            vec2_copy(&texCoors[vt3 - 1], &t.vt3);
            vec3_copy(&normals[vn1 - 1], &t.vn1);
            vec3_copy(&normals[vn2 - 1], &t.vn2);
            vec3_copy(&normals[vn3 - 1], &t.vn3);
            
            // Find the material by name and set the pointer
            t.material = find_material_by_name(mats, material_name);
            if (t.material == NULL) {
                fprintf(stderr, "Warning: Material '%s' not found\n", material_name);
            }

            mesh.triangles[mesh.count++] = t;
        }
    }

    free(vertices);
    free(normals);
    free(texCoors);
    fclose(file);

    return mesh;
}

void free_triangles(Triangles *mesh) {
    free(mesh->triangles);
}

/* checks if ray intersects triangle and stores barycentric coordinates in out*/
int ray_intersects_triangle(Ray *ray, Triangle *triangle, Vec3 *out) {
    const float epsilon = 1e-5;
    Vec3 e1, e2, e2_cross_raydir, b_cross_e1, b;
    vec3_subtract(&triangle->v2, &triangle->v1, &e1);
    vec3_subtract(&triangle->v3, &triangle->v1, &e2);
    vec3_cross(&ray->direction, &e2, &e2_cross_raydir);
    float det = vec3_dot(&e1, &e2_cross_raydir);
    if (det <= epsilon && -det <= epsilon) {
        return 0; // no solution because ray is parallel to triangle plane
    }
    float inv_det = 1.0 / det; // calculate once because div is expensive
    vec3_subtract(&ray->origin, &triangle->v1, &b);

    float u = inv_det * vec3_dot(&e2_cross_raydir, &b); // u
    if (u < 0 || u > 1.0) {
        return 0;
    }
    vec3_cross(&b, &e1, &b_cross_e1);
    float v = inv_det * vec3_dot(&ray->direction, &b_cross_e1); // v
    if (v < 0 || v + u > 1.0) {
        return 0;
    }
    float t = inv_det * vec3_dot(&e2, &b_cross_e1); // t
    if (t >= epsilon) {
        out->x = t;
        out->y = u;
        out->z = v;
        return 1;
    }
    return 0;
}

/* Can also be used for BVH in the future!!! Thanks to https://tavianator.com/2015/ray_box_nan.html */
int ray_intersects_box(Ray *ray, Vec3 *box_min, Vec3 *box_max) {
    float tmin = -INFINITY, tmax = INFINITY;
    float *bmin = (float *)box_min;
    float *bmax = (float *)box_max;
    float *rorigin = (float *)&ray->origin;
    Vec3 raydir_inv; vec3_safeinverse(&ray->direction, &raydir_inv);
    float *rdir_inv = (float *)&raydir_inv;

    for (int i = 0; i < 3; ++i) {
        float t1 = (bmin[i] - rorigin[i])*rdir_inv[i];
        float t2 = (bmax[i] - rorigin[i])*rdir_inv[i];

        tmin = max(tmin, min(t1, t2));
        tmax = min(tmax, max(t1, t2));
    }
    return tmax > max(tmin, 0.0);
}

/* checks if point is inside voxel */
int point_in_box(Vec3 *p, Vec3 *box_min, Vec3 *box_max){
    if (p->x >= box_min->x && p->x <= box_max->x &&
            p->y >= box_min->y && p->y <= box_max->y &&
            p->z >= box_min->z && p->z <= box_max->z) {
            return 1;
        }
    return 0;
}

/* function which checks if a triangle is inside a voxel. Does not always give correct result,
but if it returns zero it is guaranteed to not intersect! */
int triangle_intersects_voxel_heuristic(Triangle *t, Vec3 *voxel_min, float boxsize) {
    Vec3 voxel_max;
    vec3_add(voxel_min, &(Vec3){boxsize, boxsize, boxsize}, &voxel_max);

    // Check if any vertex is inside the voxel
    Vec3 vertices[3] = {t->v1, t->v2, t->v3};
    for (int i = 0; i < 3; i++) {
        if (point_in_box(&vertices[i], voxel_min, &voxel_max)){
            return 1;
        };
    }

    // Check if any edge intersects the voxel
    for (int i = 0; i < 3; i++) {
        Vec3 edge_start = vertices[i];
        Vec3 edge_end = vertices[(i + 1) % 3];
        Vec3 direction;
        vec3_subtract(&edge_end, &edge_start, &direction);
        
        Ray edge_ray = {edge_start, direction};
        if (ray_intersects_box(&edge_ray, voxel_min, &voxel_max)) {
            return 1;
        }
    }

    // Check if the triangle PLANE intersects the voxel
    Vec3 e1, e2, normal;
    vec3_subtract(&t->v2, &t->v1, &e1);
    vec3_subtract(&t->v3, &t->v1, &e2);
    vec3_cross(&e1, &e2, &normal); // triangle normal
    
    float d = -vec3_dot(&normal, &t->v1);
    float sign = 0;
    for (size_t i = 0; i < 8; i++)
    {
        Vec3 corner = {
            (i & 1) ? voxel_max.x : voxel_min->x,
            (i & 2) ? voxel_max.y : voxel_min->y,
            (i & 4) ? voxel_max.z : voxel_min->z,
        };
        float this_dist = vec3_dot(&normal, &corner) + d;
        if (i == 0){
            sign = this_dist <= 0 ? -1 : 1;
        }
        else{
            if ((sign < 0 && this_dist > 0) || (sign > 0 && this_dist <= 0)){
                return 1;
            }
        }
    }
    return 0;
}

/* converts 2d pixel to camera ray */
int screen2CameraDir(Camera *cam, int screenPos_x, int screenPos_y, Vec3 *result) {
    float x = (float) screenPos_x + randFloat(); // add small rand value to achieve "antialiasing"
    float y = (float) screenPos_y + randFloat();
    Vec3 cam_coor = {
        x / (float)cam->height,
        y / (float)cam->height,
        0
    };
    Vec3 center_shift = {
        -((float)cam->width / (float)cam->height) * 0.5, -0.5, -cam->focal_length
    };
    vec3_add(&cam_coor, &center_shift, result);
    vec3_normalize(result, result);
    return 1;
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

/* returns value of material property. Reads from texture if it exists */
Vec3 get_prop_val(Vec3OrTexture *vot, Triangle *triangle, Vec3 *barycentric) {
    if (vot->uses_texture) {
        return GetPixelFromTria(&vot->tex, triangle, barycentric);
    } else {
        return vot->value;
    }
}

#endif // MESH_H