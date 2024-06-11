#ifndef SPATIAL_H
#define SPATIAL_H
#include "mesh.h"

typedef struct {
    int size;
    float box_size;
    Triangle *triangles;
    int **partitioning; // cells 
} Scene;

typedef struct {
    Vec3 coor;
    float size;
} Cube;

int triangle_intersects_cube(Triangle t, Cube c){
    
}

Scene build(Triangle *triangles){
    // assumes, that every triangle coordinate is positive
    
}

#endif