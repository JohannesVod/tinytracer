A tiny but powerful raytracer in pure C. It is not as fast as for example blender cycles, but only consists of ~1000 lines of code and is easy to understand (i hope). It can handle basic materials and textures.

TODOS:
- add comments
- better datastructure for faster ray tracing (BVH instead of simple grid)
- importance sampling to reduce noise
- faster tracing using GPU acceleration
- better tone mapping (filmic, ... ?)
- SIMD instructions?