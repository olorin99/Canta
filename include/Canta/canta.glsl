#ifndef CANTA_INCLUDE_GLSL
#define CANTA_INCLUDE_GLSL

#ifndef __cplusplus

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_buffer_reference2 : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_EXT_shader_explicit_arithmetic_types : enable

#endif

#define CANTA_BINDLESS_SAMPLERS 0
#define CANTA_BINDLESS_SAMPLED_IMAGES 1
#define CANTA_BINDLESS_STORAGE_IMAGES 2
#define CANTA_BINDLESS_STORAGE_BUFFERS 3

#ifndef __cplusplus

layout (set = 0, binding = CANTA_BINDLESS_SAMPLERS) uniform sampler samplers[];

#define declareSampledImages(name, type) layout (set = 0, binding = CANTA_BINDLESS_SAMPLED_IMAGES) uniform type name[];
#define declareStorageImages(name, type, qualifiers) layout (set = 0, binding = CANTA_BINDLESS_STORAGE_IMAGES) uniform qualifiers type name[];
#define declareStorageImagesFormat(name, type, qualifiers, format) layout (format, set = 0, binding = CANTA_BINDLESS_STORAGE_IMAGES) uniform qualifiers type name[];

#define sampled1D(name, imageIndex, samplerIndex) sampler1D(name[imageIndex], samplers[samplerIndex])
#define sampled2D(name, imageIndex, samplerIndex) sampler2D(name[imageIndex], samplers[samplerIndex])
#define sampled3D(name, imageIndex, samplerIndex) sampler3D(name[imageIndex], samplers[samplerIndex])

#endif

#ifndef __cplusplus

#define declareBufferReferenceAlignQualifier(name, align, qualifiers, body) layout (scalar, buffer_reference, buffer_reference_align = align) qualifiers buffer name { body };

#else

#define declareBufferReferenceAlignQualifier(name, align, qualifiers, body) using name = u64;

#endif

#define declareBufferReferenceQualifier(name, qualifiers, body) declareBufferReferenceAlignQualifier(name, 4, qualifiers, body)
#define declareBufferReferenceAlign(name, align, body) declareBufferReferenceAlignQualifier(name, align, , body)
#define declareBufferReference(name, body) declareBufferReferenceQualifier(name, , body)
#define declareBufferReferenceReadonly(name, body) declareBufferReferenceQualifier(name, readonly, body)
#define declareBufferReferenceWriteonly(name, body) declareBufferReferenceQualifier(name, writeonly, body)

#endif