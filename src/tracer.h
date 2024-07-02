#ifndef TRACER_H
#define TRACER_H
#include "spatial.h"

Vec3 trace(Scene *scene, Ray *cam_ray, int bounces){
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
            reflect(&curr_ray, this_tria, &tria_normal, &out_reflect);
            // diffuse direction:
            Material *this_mat = this_tria->material;
            float emissive = get_prop_val(&this_mat->emissive, this_tria, &barycentric).x;
            float metallic = get_prop_val(&this_mat->metallic, this_tria, &barycentric).x;
            Vec3 color = get_prop_val(&this_mat->color, this_tria, &barycentric);

            vec3_mul(&res, &color, &res);
            if (emissive > 0){
                vec3_scale(&res, emissive, &res);
                return res;
            }
            Vec3 diffuse = rand_lambertian(&tria_normal);
            if (metallic > 0){
                vec3_lerp(&out_reflect, &diffuse, 1-metallic, &diffuse);
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

#endif