R"(#ifndef CANTA_INCLUDE_GLSL
#define CANTA_INCLUDE_GLSL

#define CANTA_USE_STORAGE_IMAGE(type, annotation, name) layout (set = 0, binding = 2) uniform annotation type name[];

#define CANTA_GET_STORAGE_IMAGE(name, index) name[index]

#endif)"