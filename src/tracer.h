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
            
            // read material properties:
            Material *this_mat = this_tria->material;
            Vec3 base_color = get_prop_val(&this_mat->color, this_tria, &barycentric);
            Vec3 spec_color = get_prop_val(&this_mat->specular_color, this_tria, &barycentric);
            float spec_ior = get_prop_val(&this_mat->specular, this_tria, &barycentric).x;
            float emissive = get_prop_val(&this_mat->emissive, this_tria, &barycentric).x;
            float metallic = get_prop_val(&this_mat->metallic, this_tria, &barycentric).x;
            float roughness = get_prop_val(&this_mat->specular_roughness, this_tria, &barycentric).x;
            // if it is light, return: 
            if (emissive > 0){
                vec3_scale(&res, emissive, &res);
                return res;
            }
            
            // reflection and diffuse:
            Vec3 tria_normal;
            GetTriangleNormal(this_tria, &barycentric, &tria_normal);
            // TODO: apply normal map here
            Vec3 new_dir = rand_lambertian(&tria_normal);
            Vec3 out_reflect; 
            reflect(&curr_ray, this_tria, &tria_normal, &out_reflect);
            vec3_lerp(&out_reflect, &new_dir, roughness, &out_reflect); // apply roughness
            float fresnel = fresnel_dielectric_cos(vec3_dot(&curr_ray.direction, &tria_normal), 1+spec_ior);
            // apply materials:
            if (randFloat() < fresnel){ // make it specular ray:
                vec3_mul(&res, &spec_color, &res); // apply specular color
                vec3_copy(&out_reflect, &new_dir);
            }
            else{
                vec3_mul(&res, &base_color, &res); // apply the base color
            }
            if (randFloat() < metallic){ // make it metallic ray:
                vec3_copy(&out_reflect, &new_dir);
            }

            // calc new ray:
            Vec3 dir_scaled; vec3_copy(&curr_ray.direction, &dir_scaled);
            vec3_scale(&dir_scaled, barycentric.x, &dir_scaled);
            vec3_add(&curr_ray.origin, &dir_scaled, &curr_ray.origin);
            vec3_copy(&new_dir, &curr_ray.direction);
        }
        else {
            break;
        }
    }
    vec3_scale(&res, 0.0, &res); // value denotes environment lighting 
    return res;
}

#endif