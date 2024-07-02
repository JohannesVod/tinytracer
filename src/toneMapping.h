#ifndef TONEMAPPING_H
#define TONEMAPPING_H
#include "tracer.h"

Vec3 pre = {0.2126f, 0.7152f, 0.0722f}; 

/* taken from https://64.github.io/tonemapping/ */
float luminance(Vec3 v)
{
    return vec3_dot(&v, &pre);
}

Vec3 change_luminance(Vec3 c_in, float l_out)
{
    float l_in = luminance(c_in);
    Vec3 res;
    vec3_scale(&c_in, (l_out/l_in), &res);
    return res;
}

Vec3 reinhard_extended_luminance(Vec3 v, float max_white_l)
{
    float l_old = luminance(v);
    float numerator = l_old * (1.0f + (l_old / (max_white_l * max_white_l)));
    float l_new = numerator / (1.0f + l_old);
    return change_luminance(v, l_new);
}

Vec3 reinhard(Vec3 v){
    Vec3 res;
    res.x = v.x + 1;
    res.y = v.y + 1;
    res.z = v.z + 1;
    vec3_inverse(&res, &res);
    vec3_mul(&res, &v, &res);
    return res;
}

#endif