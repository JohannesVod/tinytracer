#ifndef SPATIAL_H
#define SPATIAL_H
#include "mesh.h"
#include <math.h>

typedef struct {
    int *trias;
    int trias_capacity;
    int trias_count;
} Voxel;

void AddTriangle(Voxel *v, int tria_ind){
    if (v->trias_capacity == 0){
        v->trias_capacity = 2;
        v->trias = (int *)malloc(v->trias_capacity * sizeof(int));
        v->trias_count = 0;
    }
    if (v->trias_count >= v->trias_capacity){
        v->trias_capacity *= 2;
        v->trias = (int *)realloc(v->trias, v->trias_capacity * sizeof(int));
    }
    v->trias[v->trias_count] = tria_ind;
    v->trias_count += 1;
}

void freeVoxel(Voxel *v){
    free(v->trias);
}

typedef struct {
    Vec3 p1;
    Vec3 p2;
} Box;

typedef struct {
    Vec3Int numboxes; // number of boxes on all dimensions
    float boxsize;
    Triangle *triangles;
    Voxel *voxels; // cells
    Box bbox;
} Scene;

int getVoxelIndex(Scene *scene, int x, int y, int z){
    return z*scene->numboxes.x*scene->numboxes.y + y*scene->numboxes.x + x;
}

void freeScene(Scene *scene){
    // initialize voxels:
    for (int x_i = 0; x_i < scene->numboxes.x; x_i++){
        for (int y_i = 0; y_i < scene->numboxes.y; y_i++){
            for (int z_i = 0; z_i < scene->numboxes.z; z_i++){
                int arr_ind = getVoxelIndex(scene, x_i, y_i, z_i);
                freeVoxel(&scene->voxels[arr_ind]);
            }
        }
    }
    free(scene->voxels);
    free(scene->triangles);
}

void point2floor(Vec3 *p, float boxsize){
    vec3_scale(p, 1/boxsize, p);
    vec3_floor(p, p);
    vec3_scale(p, boxsize, p);
}

void point2ceil(Vec3 *p, float boxsize){
    vec3_scale(p, 1/boxsize, p);
    vec3_ceil(p, p);
    vec3_scale(p, boxsize, p);
}

/* Calculates the gridcell corresponding to a point. Returns cell id as Vec3Int */
Vec3Int point2voxel(Scene *scene, Vec3 *point){
    Vec3Int coor = {0, 0, 0};
    Vec3 scaled_p; 
    vec3_subtract(point, &scene->bbox.p1, &scaled_p); // shift grid to zero
    vec3_scale(&scaled_p, 1/scene->boxsize, &scaled_p);
    vec3_floor(&scaled_p, &scaled_p);
    vec3_2int(&scaled_p, &coor);
    return coor;
}

Box get_bbox(Triangle *t){
    Vec3 min_p;
    Vec3 max_p;
    vec3_copy(&t->v1, &min_p);
    vec3_copy(&t->v1, &max_p);
    Vec3 vertices[3] = {t->v2, t->v3};
    for (int j = 0; j < 2; j++) {
        Vec3 v = vertices[j];
        // Update min coordinates
        if (v.x < min_p.x) min_p.x = v.x;
        if (v.y < min_p.y) min_p.y = v.y;
        if (v.z < min_p.z) min_p.z = v.z;

        // Update max coordinates
        if (v.x > max_p.x) max_p.x = v.x;
        if (v.y > max_p.y) max_p.y = v.y;
        if (v.z > max_p.z) max_p.z = v.z;
    }
    Box bbox = {{0, 0, 0}, {0, 0, 0}};
    vec3_copy(&min_p, &bbox.p1);
    vec3_copy(&max_p, &bbox.p2);
    return bbox;
}

void buildScene(Triangle *triangles, int num_trias, Scene *scene, int desired_boxes){
    scene->triangles = triangles;
    // calculate total bounding box first:
    vec3_copy(&triangles[0].v1, &scene->bbox.p1);
    vec3_copy(&triangles[0].v1, &scene->bbox.p2);

    for (int i = 0; i < num_trias; i++) {
        Triangle t = triangles[i];

        // Check each vertex of the triangle
        Vec3 vertices[3] = {t.v1, t.v2, t.v3};
        for (int j = 0; j < 3; j++) {
            Vec3 v = vertices[j];

            // Update min coordinates
            if (v.x < scene->bbox.p1.x) scene->bbox.p1.x = v.x;
            if (v.y < scene->bbox.p1.y) scene->bbox.p1.y = v.y;
            if (v.z < scene->bbox.p1.z) scene->bbox.p1.z = v.z;

            // Update max coordinates
            if (v.x > scene->bbox.p2.x) scene->bbox.p2.x = v.x;
            if (v.y > scene->bbox.p2.y) scene->bbox.p2.y = v.y;
            if (v.z > scene->bbox.p2.z) scene->bbox.p2.z = v.z;
        }
    }
    // snap bounding box to grid:
    scene->boxsize = (scene->bbox.p2.x - scene->bbox.p1.x)/desired_boxes;
    point2floor(&scene->bbox.p1, scene->boxsize);
    point2ceil(&scene->bbox.p2, scene->boxsize);
    Vec3 diff; vec3_subtract(&scene->bbox.p2, &scene->bbox.p1, &diff);
    Vec3 scaled; vec3_scale(&diff, 1/scene->boxsize, &scaled);
    vec3_round(&scaled, &scaled);
    // calculate number of boxes per dimension:
    vec3_2int(&scaled, &scene->numboxes);
    scene->voxels = (Voxel *)malloc(scene->numboxes.x*scene->numboxes.y*scene->numboxes.z * sizeof(Voxel));
    // initialize voxels:
    for (int x_i = 0; x_i < scene->numboxes.x; x_i++){
        for (int y_i = 0; y_i < scene->numboxes.y; y_i++){
            for (int z_i = 0; z_i < scene->numboxes.z; z_i++){
                Voxel voxel = {
                    .trias = NULL,
                    .trias_capacity = 0,
                    .trias_count = 0,
                };
                int arr_ind = getVoxelIndex(scene, x_i, y_i, z_i);
                scene->voxels[arr_ind] = voxel;
            }
        }
    }

    for (int i = 0; i < num_trias; i++) {
        Triangle t = triangles[i];
        Box bbox = get_bbox(&t);
        Vec3Int coor_1 = point2voxel(scene, &bbox.p1);
        Vec3Int coor_2 = point2voxel(scene, &bbox.p2);
        for (int x_i = coor_1.x; x_i <= coor_2.x; x_i++){
            for (int y_i = coor_1.y; y_i <= coor_2.y; y_i++){
                for (int z_i = coor_1.z; z_i <= coor_2.z; z_i++){
                    int arr_ind = getVoxelIndex(scene, x_i, y_i, z_i);
                    Voxel *vox = &scene->voxels[arr_ind];
                    AddTriangle(vox, i);
                }
            }
        }
    }
}

void test(Scene *scene){
    for (int x_i = 0; x_i < scene->numboxes.x; x_i++){
        for (int y_i = 0; y_i < scene->numboxes.y; y_i++){
            for (int z_i = 0; z_i < scene->numboxes.z; z_i++){
                int arr_ind = getVoxelIndex(scene, x_i, y_i, z_i);
                Voxel *vox = &scene->voxels[arr_ind];
                float box_pos_x = scene->bbox.p1.x + scene->boxsize*x_i;
                float box_pos_y = scene->bbox.p1.y + scene->boxsize*y_i;
                float box_pos_z = scene->bbox.p1.z + scene->boxsize*z_i;
                if (vox->trias_count != 0){
                    printf("%f, %f, %f|triangle ind: %d\n", box_pos_x, box_pos_y, box_pos_z, vox->trias[0]);
                }
            }
        }
    }
}

int isInGrid(Scene *scene, Vec3Int *cell){
    if (cell->x < 0 || cell->y < 0 || cell->z < 0){
        return 0;
    }
    if (cell->x > scene->numboxes.x || cell->y > scene->numboxes.y || cell->z > scene->numboxes.z){
        return 0;
    }
    return 1;
}

/*casts a ray into the scene. Returns the index of the triangle it intersects.*/
void castRay(Ray *r, Scene *scene){
    Vec3Int curr_cell = point2voxel(scene, &r->origin);
    vec3_fix(&r->direction);
    
    Vec3Int directions = {1, 1, 1}; 
    if (r->direction.x < 0){ directions.x = -1;}
    if (r->direction.y < 0){ directions.y = -1;}
    if (r->direction.z < 0){ directions.z = -1;}

    // calculate initial tmax as in http://www.cse.yorku.ca/~amana/research/grid.pdf
    Vec3 ceiled, floored; 
    vec3_ceil(&r->origin, &ceiled);
    vec3_floor(&r->origin, &floored);
    vec3_subtract(&ceiled, &r->origin, &ceiled);
    vec3_subtract(&floored, &r->origin, &floored);
    Vec3 tDelta;
    vec3_copy(&r->direction, &tDelta);
    vec3_inverse(&tDelta, &tDelta);
    vec3_mul(&ceiled, &tDelta, &ceiled);
    vec3_mul(&floored, &tDelta, &floored);
    
    Vec3 tMax = ceiled;
    if (tMax.x < 0){ tMax.x = floored.x; }
    if (tMax.y < 0){ tMax.y = floored.y; }
    if (tMax.z < 0){ tMax.z = floored.z; }

    vec3_abs(&tDelta, &tDelta);

    while (isInGrid(scene, &curr_cell)){
        // handle cell here:
        // handleCell(&curr_cell);
        printf("cell step: %d, %d, %d \n", curr_cell.x, curr_cell.y, curr_cell.z);
        if (tMax.x < tMax.y){
            if (tMax.x < tMax.z){
                tMax.x += tDelta.x;
                curr_cell.x += directions.x;
            }
            else{
                tMax.z += tDelta.z;
                curr_cell.z += directions.z;
            }
        }
        else{
            if (tMax.y < tMax.z){
                tMax.y += tDelta.y;
                curr_cell.y += directions.y;
            }
            else{
                tMax.z += tDelta.z;
                curr_cell.z += directions.z;
            }
        }
    }
}

#endif