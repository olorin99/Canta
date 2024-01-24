R""(#ifndef CANTA_INCLUDE_GLSL
#define CANTA_INCLUDE_GLSL

#define CANTA_BINDLESS_SAMPLERS 0
#define CANTA_BINDLESS_SAMPLED_IMAGES 1
#define CANTA_BINDLESS_STORAGE_IMAGES 2
#define CANTA_BINDLESS_STORAGE_BUFFERS 3

#define CANTA_USE_STORAGE_IMAGE(type, annotation, name) layout (set = 0, binding = CANTA_BINDLESS_STORAGE_IMAGES) uniform annotation type name[];

#define CANTA_GET_STORAGE_IMAGE(name, index) name[index]

#endif)""