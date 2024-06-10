#ifndef SPATIAL_H
#define SPATIAL_H
#include "mesh.h"

typedef struct {
    int size;
    float box_size;
    Triangle *triangles;
    int **partitioning; // cells 
} Scene;

Scene build(Triangle *triangles){
    // assumes, that every triangle coordinate is positive
    
}

#endif