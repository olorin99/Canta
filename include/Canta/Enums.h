#ifndef CANTA_ENUMS_H
#define CANTA_ENUMS_H

#include <volk.h>
#include <type_traits>

namespace canta {

    enum class Error {
        HOST_MEMORY = VK_ERROR_OUT_OF_HOST_MEMORY,
        DEVICE_MEMORY = VK_ERROR_OUT_OF_DEVICE_MEMORY,
        DEVICE_LOST = VK_ERROR_DEVICE_LOST,
        EXTENSION_NOT_PRESENT = VK_ERROR_EXTENSION_NOT_PRESENT,
        FEATURE_NOT_PRESENT = VK_ERROR_FEATURE_NOT_PRESENT,

        INVALID_GPU = 0x1000000,
        INVALID_PLATFORM = 0x200000
    };

    enum class PhysicalDeviceType {
        OTHER = VK_PHYSICAL_DEVICE_TYPE_OTHER,
        INTEGRATED = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU,
        DISCRETE = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,
        VIRTUAL = VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,
        CPU = VK_PHYSICAL_DEVICE_TYPE_CPU,
    };

    enum class QueueType {
        NONE = 0,
        GRAPHICS = VK_QUEUE_GRAPHICS_BIT,
        COMPUTE = VK_QUEUE_COMPUTE_BIT,
        TRANSFER = VK_QUEUE_TRANSFER_BIT,
        SPARSE_BINDING = VK_QUEUE_SPARSE_BINDING_BIT,
        PRESENT = 0x20
    };

    enum class PresentMode {
        IMMEDIATE = VK_PRESENT_MODE_IMMEDIATE_KHR,
        MAILBOX = VK_PRESENT_MODE_MAILBOX_KHR,
        FIFO = VK_PRESENT_MODE_FIFO_KHR,
        FIFO_RELAXED = VK_PRESENT_MODE_FIFO_RELAXED_KHR,
        SHARED_DEMAND = VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR,
        SHARED_CONTINUOUS = VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR
    };

    enum class Format {
        UNDEFINED = VK_FORMAT_UNDEFINED,
        RG4_UNORM = VK_FORMAT_R4G4_UNORM_PACK8,
        RGBA4_UNORM = VK_FORMAT_R4G4B4A4_UNORM_PACK16,
        BGRA4_UNORM = VK_FORMAT_B4G4R4A4_UNORM_PACK16,
        RGB5_UNORM = VK_FORMAT_R5G6B5_UNORM_PACK16,
        BGR5_UNORM = VK_FORMAT_B5G6R5_UNORM_PACK16,
        RGB5A1_UNORM = VK_FORMAT_R5G5B5A1_UNORM_PACK16,
        BGR5A1_UNORM = VK_FORMAT_B5G5R5A1_UNORM_PACK16,
        A1RGB5_UNORM = VK_FORMAT_A1R5G5B5_UNORM_PACK16,
        R8_UNORM = VK_FORMAT_R8_UNORM,
        R8_SNORM = VK_FORMAT_R8_SNORM,
        R8_USCALED = VK_FORMAT_R8_USCALED,
        R8_SSCALED = VK_FORMAT_R8_SSCALED,
        R8_UINT = VK_FORMAT_R8_UINT,
        R8_SINT = VK_FORMAT_R8_SINT,
        R8_SRGB = VK_FORMAT_R8_SRGB,
        RG8_UNORM = VK_FORMAT_R8G8_UNORM,
        RG8_SNORM = VK_FORMAT_R8G8_SNORM,
        RG8_USCALED = VK_FORMAT_R8G8_USCALED,
        RG8_SSCALED = VK_FORMAT_R8G8_SSCALED,
        RG8_UINT = VK_FORMAT_R8G8_UINT,
        RG8_SINT = VK_FORMAT_R8G8_SINT,
        RG8_SRGB = VK_FORMAT_R8G8_SRGB,
        RGB8_UNORM = VK_FORMAT_R8G8B8_UNORM,
        RGB8_SNORM = VK_FORMAT_R8G8B8_SNORM,
        RGB8_USCALED = VK_FORMAT_R8G8B8_USCALED,
        RGB8_SSCALED = VK_FORMAT_R8G8B8_SSCALED,
        RGB8_UINT = VK_FORMAT_R8G8B8_UINT,
        RGB8_SINT = VK_FORMAT_R8G8B8_SINT,
        RGB8_SRGB = VK_FORMAT_R8G8B8_SRGB,
        BGR8_UNORM = VK_FORMAT_B8G8R8_UNORM,
        BGR8_SNORM = VK_FORMAT_B8G8R8_SNORM,
        BGR8_USCALED = VK_FORMAT_B8G8R8_USCALED,
        BGR8_SSCALED = VK_FORMAT_B8G8R8_SSCALED,
        BGR8_UINT = VK_FORMAT_B8G8R8_UINT,
        BGR8_SINT = VK_FORMAT_B8G8R8_SINT,
        BGR8_SRGB = VK_FORMAT_B8G8R8_SRGB,
        RGBA8_UNORM = VK_FORMAT_R8G8B8A8_UNORM,
        RGBA8_SNORM = VK_FORMAT_R8G8B8A8_SNORM,
        RGBA8_USCALED = VK_FORMAT_R8G8B8A8_USCALED,
        RGBA8_SSCALED = VK_FORMAT_R8G8B8A8_SSCALED,
        RGBA8_UINT = VK_FORMAT_R8G8B8A8_UINT,
        RGBA8_SINT = VK_FORMAT_R8G8B8A8_SINT,
        RGBA8_SRGB = VK_FORMAT_R8G8B8A8_SRGB,
        BGRA8_UNORM = VK_FORMAT_B8G8R8A8_UNORM,
        BGRA8_SNORM = VK_FORMAT_B8G8R8A8_SNORM,
        BGRA8_USCALED = VK_FORMAT_B8G8R8A8_USCALED,
        BGRA8_SSCALED = VK_FORMAT_B8G8R8A8_SSCALED,
        BGRA8_UINT = VK_FORMAT_B8G8R8A8_UINT,
        BGRA8_SINT = VK_FORMAT_B8G8R8A8_SINT,
        BGRA8_SRGB = VK_FORMAT_B8G8R8A8_SRGB,
        ABGR8_UNORM = VK_FORMAT_A8B8G8R8_UNORM_PACK32,
        ABGR8_SNORM = VK_FORMAT_A8B8G8R8_SNORM_PACK32,
        ABGR8_USCALED = VK_FORMAT_A8B8G8R8_USCALED_PACK32,
        ABGR8_SSCALED = VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
        ABGR8_UINT = VK_FORMAT_A8B8G8R8_UINT_PACK32,
        ABGR8_SINT = VK_FORMAT_A8B8G8R8_SINT_PACK32,
        ABGR8_SRGB = VK_FORMAT_A8B8G8R8_SRGB_PACK32,
        A2RGB10_UNORM = VK_FORMAT_A2R10G10B10_UNORM_PACK32,
        A2RGB10_SNORM = VK_FORMAT_A2R10G10B10_SNORM_PACK32,
        A2RGB10_USCALED = VK_FORMAT_A2R10G10B10_USCALED_PACK32,
        A2RGB10_SSCALED = VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
        A2RGB10_UINT = VK_FORMAT_A2R10G10B10_UINT_PACK32,
        A2RGB10_SINT = VK_FORMAT_A2R10G10B10_SINT_PACK32,
        A2BGR10_UNORM = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        A2BGR10_SNORM = VK_FORMAT_A2B10G10R10_SNORM_PACK32,
        A2BGR10_USCALED = VK_FORMAT_A2B10G10R10_USCALED_PACK32,
        A2BGR10_SSCALED = VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
        A2BGR10_UINT = VK_FORMAT_A2B10G10R10_UINT_PACK32,
        A2BGR10_SINT = VK_FORMAT_A2B10G10R10_SINT_PACK32,
        R16_UNORM = VK_FORMAT_R16_UNORM,
        R16_SNORM = VK_FORMAT_R16_SNORM,
        R16_USCALED = VK_FORMAT_R16_USCALED,
        R16_SSCALED = VK_FORMAT_R16_SSCALED,
        R16_UINT = VK_FORMAT_R16_UINT,
        R16_SINT = VK_FORMAT_R16_SINT,
        R16_SFLOAT = VK_FORMAT_R16_SFLOAT,
        RG16_UNORM = VK_FORMAT_R16G16_UNORM,
        RG16_SNORM = VK_FORMAT_R16G16_SNORM,
        RG16_USCALED = VK_FORMAT_R16G16_USCALED,
        RG16_SSCALED = VK_FORMAT_R16G16_SSCALED,
        RG16_UINT = VK_FORMAT_R16G16_UINT,
        RG16_SINT = VK_FORMAT_R16G16_SINT,
        RG16_SFLOAT = VK_FORMAT_R16G16_SFLOAT,
        RGB16_UNORM = VK_FORMAT_R16G16B16_UNORM,
        RGB16_SNORM = VK_FORMAT_R16G16B16_SNORM,
        RGB16_USCALED = VK_FORMAT_R16G16B16_USCALED,
        RGB16_SSCALED = VK_FORMAT_R16G16B16_SSCALED,
        RGB16_UINT = VK_FORMAT_R16G16B16_UINT,
        RGB16_SINT = VK_FORMAT_R16G16B16_SINT,
        RGB16_SFLOAT = VK_FORMAT_R16G16B16_SFLOAT,
        RGBA16_UNORM = VK_FORMAT_R16G16B16A16_UNORM,
        RGBA16_SNORM = VK_FORMAT_R16G16B16A16_SNORM,
        RGBA16_USCALED = VK_FORMAT_R16G16B16A16_USCALED,
        RGBA16_SSCALED = VK_FORMAT_R16G16B16A16_SSCALED,
        RGBA16_UINT = VK_FORMAT_R16G16B16A16_UINT,
        RGBA16_SINT = VK_FORMAT_R16G16B16A16_SINT,
        RGBA16_SFLOAT = VK_FORMAT_R16G16B16A16_SFLOAT,
        R32_UINT = VK_FORMAT_R32_UINT,
        R32_SINT = VK_FORMAT_R32_SINT,
        R32_SFLOAT = VK_FORMAT_R32_SFLOAT,
        RG32_UINT = VK_FORMAT_R32G32_UINT,
        RG32_SINT = VK_FORMAT_R32G32_SINT,
        RG32_SFLOAT = VK_FORMAT_R32G32_SFLOAT,
        RGB32_UINT = VK_FORMAT_R32G32B32_UINT,
        RGB32_SINT = VK_FORMAT_R32G32B32_SINT,
        RGB32_SFLOAT = VK_FORMAT_R32G32B32_SFLOAT,
        RGBA32_UINT = VK_FORMAT_R32G32B32A32_UINT,
        RGBA32_SINT = VK_FORMAT_R32G32B32A32_SINT,
        RGBA32_SFLOAT = VK_FORMAT_R32G32B32A32_SFLOAT,
        R64_UINT = VK_FORMAT_R64_UINT,
        R64_SINT = VK_FORMAT_R64_SINT,
        R64_SFLOAT = VK_FORMAT_R64_SFLOAT,
        RG64_UINT = VK_FORMAT_R64G64_UINT,
        RG64_SINT = VK_FORMAT_R64G64_SINT,
        RG64_SFLOAT = VK_FORMAT_R64G64_SFLOAT,
        RGB64_UINT = VK_FORMAT_R64G64B64_UINT,
        RGB64_SINT = VK_FORMAT_R64G64B64_SINT,
        RGB64_SFLOAT = VK_FORMAT_R64G64B64_SFLOAT,
        RGBA64_UINT = VK_FORMAT_R64G64B64A64_UINT,
        RGBA64_SINT = VK_FORMAT_R64G64B64A64_SINT,
        RGBA64_SFLOAT = VK_FORMAT_R64G64B64A64_SFLOAT,
        B10GR11_UFLOAT = VK_FORMAT_B10G11R11_UFLOAT_PACK32,
        E5BGR9_UFLOAT = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
        D16_UNORM = VK_FORMAT_D16_UNORM,
        X8D24_UNORM = VK_FORMAT_X8_D24_UNORM_PACK32,
        D32_SFLOAT = VK_FORMAT_D32_SFLOAT,
        S8_UINT = VK_FORMAT_S8_UINT,
        D16_UNORM_S8_UINT = VK_FORMAT_D16_UNORM_S8_UINT,
        D24_UNORM_S8_UINT = VK_FORMAT_D24_UNORM_S8_UINT,
        D32_SFLOAT_S8_UINT = VK_FORMAT_D32_SFLOAT_S8_UINT,
        BC1_RGB_UNORM = VK_FORMAT_BC1_RGB_UNORM_BLOCK,
        BC1_RGB_SRGB = VK_FORMAT_BC1_RGB_SRGB_BLOCK,
        BC1_RGBA_UNORM = VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
        BC1_RGBA_SRGB = VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
        BC2_UNORM = VK_FORMAT_BC2_UNORM_BLOCK,
        BC2_SRGB = VK_FORMAT_BC2_SRGB_BLOCK,
        BC3_UNORM = VK_FORMAT_BC3_UNORM_BLOCK,
        BC3_SRGB = VK_FORMAT_BC3_SRGB_BLOCK,
        BC4_UNORM = VK_FORMAT_BC4_UNORM_BLOCK,
        BC4_SNORM = VK_FORMAT_BC4_SNORM_BLOCK,
        BC5_UNORM = VK_FORMAT_BC5_UNORM_BLOCK,
        BC5_SNORM = VK_FORMAT_BC5_SNORM_BLOCK,
        BC6_UFLOAT = VK_FORMAT_BC6H_UFLOAT_BLOCK,
        BC6_SFLOAT = VK_FORMAT_BC6H_SFLOAT_BLOCK,
        BC7_UNORM = VK_FORMAT_BC7_UNORM_BLOCK,
        BC7_SRGB = VK_FORMAT_BC7_SRGB_BLOCK,
        ETC2_RGB8_UNORM = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
        ETC2_RGB8_SRGB = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
        ETC2_RGB8A1_UNORM = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
        ETC2_RGB8A1_SRGB = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
        ETC2_RGBA8_UNORM = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
        ETC2_RGBA8_SRGB = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
        EAC_R11_UNORM = VK_FORMAT_EAC_R11_UNORM_BLOCK,
        EAC_R11_SNORM = VK_FORMAT_EAC_R11_SNORM_BLOCK,
        EAC_RG11_UNORM = VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
        EAC_RG11_SNORM = VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
        ASTC_44_UNORM = VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
        ASTC_44_SRGB = VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
        ASTC_54_UNORM = VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
        ASTC_54_SRGB = VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
        ASTC_55_UNORM = VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
        ASTC_55_SRGB = VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
        ASTC_65_UNORM = VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
        ASTC_65_SRGB = VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
        ASTC_66_UNORM = VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
        ASTC_66_SRGB = VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
        ASTC_85_UNORM = VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
        ASTC_85_SRGB = VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
        ASTC_86_UNORM = VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
        ASTC_86_SRGB = VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
        ASTC_88_UNORM = VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
        ASTC_88_SRGB = VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
        ASTC_105_UNORM = VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
        ASTC_105_SRGB = VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
        ASTC_106_UNORM = VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
        ASTC_106_SRGB = VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
        ASTC_108_UNORM = VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
        ASTC_108_SRGB = VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
        ASTC_1010_UNORM = VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
        ASTC_1010_SRGB = VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
        ASTC_1210_UNORM = VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
        ASTC_1210_SRGB = VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
        ASTC_1212_UNORM = VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
        ASTC_1212_SRGB = VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
        //TODO: rest of these
        VK_FORMAT_G8B8G8R8_422_UNORM,
        VK_FORMAT_B8G8R8G8_422_UNORM,
        VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
        VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
        VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
        VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
        VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
        VK_FORMAT_R10X6_UNORM_PACK16,
        VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
        VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
        VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
        VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
        VK_FORMAT_R12X4_UNORM_PACK16,
        VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
        VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
        VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
        VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
        VK_FORMAT_G16B16G16R16_422_UNORM,
        VK_FORMAT_B16G16R16G16_422_UNORM,
        VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
        VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
        VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
        VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
        VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
        VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
        VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
        VK_FORMAT_A4R4G4B4_UNORM_PACK16,
        VK_FORMAT_A4B4G4R4_UNORM_PACK16,
        VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
        VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
        VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
        VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
        VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
        VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
        VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
        VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
        VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
        VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
        VK_FORMAT_R16G16_S10_5_NV,
        VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR,
        VK_FORMAT_A8_UNORM_KHR,
        VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT,
        VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT,
        VK_FORMAT_G8B8G8R8_422_UNORM_KHR,
        VK_FORMAT_B8G8R8G8_422_UNORM_KHR,
        VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR,
        VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR,
        VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR,
        VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR,
        VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR,
        VK_FORMAT_R10X6_UNORM_PACK16_KHR,
        VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR,
        VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR,
        VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR,
        VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR,
        VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR,
        VK_FORMAT_R12X4_UNORM_PACK16_KHR,
        VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR,
        VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR,
        VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR,
        VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR,
        VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR,
        VK_FORMAT_G16B16G16R16_422_UNORM_KHR,
        VK_FORMAT_B16G16R16G16_422_UNORM_KHR,
        VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR,
        VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR,
        VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR,
        VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR,
        VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM_KHR,
        VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT,
        VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT,
        VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT,
        VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT,
        VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT,
        VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT
    };

    enum class ImageLayout {
        UNDEFINED = VK_IMAGE_LAYOUT_UNDEFINED,
        GENERAL = VK_IMAGE_LAYOUT_GENERAL,
        COLOUR_ATTACHMENT = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        DEPTH_STENCIL_READ_ONLY = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        SHADER_READ_ONLY = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        TRANSFER_SRC = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        TRANSFER_DST = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        PREINITIALISED = VK_IMAGE_LAYOUT_PREINITIALIZED,
        DEPTH_READ_ONLY_STENCIL_ATTACHMENT = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        DEPTH_ATTACHMENT_STENCIL_READ_ONLY = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        DEPTH_ATTACHMENT = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        DEPTH_READ_ONLY = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        STENCIL_ATTACHMENT = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
        STENCIL_READ_ONLY = VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        READ_ONLY = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        ATTACHMENT = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
        PRESENT = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VIDEO_DECODE_DST = VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR,
        VIDEO_DECODE_SRC = VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR,
        VIDEO_DECODE_DPB = VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR,
        SHARED_PRESENT = VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR,
        FRAGMENT_DENSITY_MAP = VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT,
        FRAGMENT_SHADING_RATE_ATTACHMENT = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
        VIDEO_ENCODE_DST = VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR,
        VIDEO_ENCODE_SRC = VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR,
        VIDEO_ENCODE_DPB = VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR,
        FEEDBACK_LOOP = VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT,
    };

    enum class ImageUsage {
        TRANSFER_SRC = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        TRANSFER_DST = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        SAMPLED = VK_IMAGE_USAGE_SAMPLED_BIT,
        STORAGE = VK_IMAGE_USAGE_STORAGE_BIT,
        COLOUR_ATTACHMENT = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        DEPTH_STENCIL_ATTACHMENT = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        TRANSIENT_ATTACHMENT = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
        INPUT_ATTACHMENT = VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VIDEO_DECODE_DST = VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR,
        VIDEO_DECODE_SRC = VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
        VIDEO_DECODE_DPB = VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR,
        FRAGMENT_DENSITY_MAP = VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT,
        FRAGMENT_SHADING_RATE_ATTACHMENT = VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
        HOST_TRANSFER = VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT,
        VIDEO_ENCODE_DST = VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR,
        VIDEO_ENCODE_SRC = VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
        VIDEO_ENCODE_DPB = VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR,
        ATTACHMENT_FEEDBACK_LOOP = VK_IMAGE_USAGE_ATTACHMENT_FEEDBACK_LOOP_BIT_EXT,
    };

    enum class BufferUsage {
        TRANSFER_SRC = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        TRANSFER_DST = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        UNIFORM_TEXEL = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT,
        STORAGE_TEXEL = VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT,
        UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        STORAGE = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        INDIRECT = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
        DEVICE_ADDRESS = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VIDEO_DECODE_SRC = VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR,
        VIDEO_DECODE_DST = VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR,
        TRANSFORM_FEEDBACK = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT,
        TRANSFORM_FEEDBACK_COUNTER = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT,
        CONDITIONAL_RENDERING = VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT,
        ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
        ACCELERATION_STRUCTURE_STORAGE = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
        SHADER_BINDING_TABLE = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
        VIDEO_ENCODE_DST = VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR,
        VIDEO_ENCODE_SRC = VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR,
        SAMPLER_DESCRIPTOR = VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT,
        RESOURCE_DESCRIPTOR = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT,
        PUSH_DESCRIPTOR = VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT,
        MICROMAP_BUILD_INPUT_READ_ONLY = VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT,
        MICROMAP_STORAGE = VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT,
    };

    enum class MemoryType {
        STAGING = 0,
        DEVICE = 1,
        READBACK = 2
    };

    enum class ImageType {
        AUTO = 3,
        IMAGE1D = VK_IMAGE_TYPE_1D,
        IMAGE2D = VK_IMAGE_TYPE_2D,
        IMAGE3D = VK_IMAGE_TYPE_3D,
    };

    enum class ImageViewType {
        AUTO = 7,
        VIEW1D = VK_IMAGE_VIEW_TYPE_1D,
        VIEW2D = VK_IMAGE_VIEW_TYPE_2D,
        VIEW3D = VK_IMAGE_VIEW_TYPE_3D,
        VIEW_CUBE = VK_IMAGE_VIEW_TYPE_CUBE,
        VIEW1D_ARRAY = VK_IMAGE_VIEW_TYPE_1D_ARRAY,
        VIEW2D_ARRAY = VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        VIEW3D_ARRAY = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY,
    };

    enum class ShaderStage {
        NONE = 0,
        VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
        TESS_CONTROL = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        TESS_EVAL = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT,
        FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
        COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT,
        ALL_GRAPHICS = VK_SHADER_STAGE_ALL_GRAPHICS,
        ALL = VK_SHADER_STAGE_ALL,
        RAYGEN = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
        ANY_HIT = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
        CLOSEST_HIT = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
        MISS = VK_SHADER_STAGE_MISS_BIT_KHR,
        INTERSECTION = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
        CALLABLE = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
        TASK = VK_SHADER_STAGE_TASK_BIT_EXT,
        MESH = VK_SHADER_STAGE_MESH_BIT_EXT
    };

    enum class PipelineStage {
        TOP = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        DRAW_INDIRECT = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
        VERTEX_INPUT = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VERTEX_SHADER = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
        TESS_CONTROL_SHADER = VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT,
        TESS_EVAL_SHADER = VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT,
        GEOMETRY_SHADER = VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT,
        FRAGMENT_SHADER = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        EARLY_FRAGMENT_TEST = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        LATE_FRAGMENT_TEST = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        COLOUR_OUTPUT = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        COMPUTE_SHADER = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        TRANSFER = VK_PIPELINE_STAGE_TRANSFER_BIT,
        BOTTOM = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        HOST = VK_PIPELINE_STAGE_HOST_BIT,
        ALL_GRAPHICS = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
        ALL_COMMANDS = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        NONE = VK_PIPELINE_STAGE_NONE,
        FEEDBACK = VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT,
        CONDITIONAL_RENDERING = VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT,
        ACCELERATION_STRCUTURE_BUILD = VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
        RAY_TRACING_SHADER = VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
        FRAGMENT_DENSITY_PROCESS = VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT,
        FRAGMENT_SHADING_RATE_ATTACHMENT = VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
        COMMAND_PREPROCSESS = VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV,
        TASK_SHADER = VK_PIPELINE_STAGE_TASK_SHADER_BIT_EXT,
        MESH_SHADER = VK_PIPELINE_STAGE_MESH_SHADER_BIT_EXT,
    };

    enum class Access {
        INDIRECT = VK_ACCESS_INDIRECT_COMMAND_READ_BIT,
        INDEX_READ = VK_ACCESS_INDEX_READ_BIT,
        VERTEX_ATTRIBUTE_READ = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
        UNIFORM = VK_ACCESS_UNIFORM_READ_BIT,
        INPUT_ATTACHMENT = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT,
        SHADER_READ = VK_ACCESS_SHADER_READ_BIT,
        SHADER_WRITE = VK_ACCESS_SHADER_WRITE_BIT,
        COLOUR_READ = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        COLOUR_WRITE = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        DEPTH_STENCIL_READ = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        DEPTH_STENCIL_WRITE = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        TRANSFER_READ = VK_ACCESS_TRANSFER_READ_BIT,
        TRANSFER_WRITE = VK_ACCESS_TRANSFER_WRITE_BIT,
        HOST_READ = VK_ACCESS_HOST_READ_BIT,
        HOST_WRITE = VK_ACCESS_HOST_WRITE_BIT,
        MEMORY_READ = VK_ACCESS_MEMORY_READ_BIT,
        MEMORY_WRITE = VK_ACCESS_MEMORY_WRITE_BIT,
        NONE = VK_ACCESS_NONE,
        TRANSFORM_FEEDBACK_WRITE = VK_ACCESS_TRANSFORM_FEEDBACK_WRITE_BIT_EXT,
        TRANSFORM_FEEDBACK_COUNTER_READ = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT,
        TRANSFORM_FEEDBACK_COUNTER_WRITE = VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT,
        CONDITIONAL_RENDERING_READ = VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT,
        COLOUR_READ_NONCOHERENT = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT,
        ACCELERATION_STRUCTURE_READ = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        ACCELERATION_STRUCTURE_WRITE = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
        FRAGMENT_DENSITY_MAP_READ = VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT,
        FRAGMENT_SHADING_RATE_READ = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR,
    };

    enum class CullMode {
        NONE = VK_CULL_MODE_NONE,
        FRONT = VK_CULL_MODE_FRONT_BIT,
        BACK = VK_CULL_MODE_BACK_BIT,
        FRONT_BACK = VK_CULL_MODE_FRONT_AND_BACK,
    };

    enum class FrontFace {
        CCW = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        CW = VK_FRONT_FACE_CLOCKWISE
    };

    enum class PolygonMode {
        FILL = VK_POLYGON_MODE_FILL,
        LINE = VK_POLYGON_MODE_LINE,
        POINT = VK_POLYGON_MODE_POINT,
    };

    enum class CompareOp {
        NEVER = VK_COMPARE_OP_NEVER,
        LESS = VK_COMPARE_OP_LESS,
        EQUAL = VK_COMPARE_OP_EQUAL,
        LEQUAL = VK_COMPARE_OP_LESS_OR_EQUAL,
        GREATER = VK_COMPARE_OP_GREATER,
        NEQUAL = VK_COMPARE_OP_NOT_EQUAL,
        GEQUAL = VK_COMPARE_OP_GREATER_OR_EQUAL,
        ALWAYS = VK_COMPARE_OP_ALWAYS,
    };

    enum class BlendFactor {
        ZERO = VK_BLEND_FACTOR_ZERO,
        ONE = VK_BLEND_FACTOR_ONE,
        SRC_COLOUR = VK_BLEND_FACTOR_SRC_COLOR,
        ONE_MINUS_SRC_COLOUR = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
        DST_COLOUR = VK_BLEND_FACTOR_DST_COLOR,
        ONE_MINUS_DST_COLOUR = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
        SRC_ALPHA = VK_BLEND_FACTOR_SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        DST_ALPHA = VK_BLEND_FACTOR_DST_ALPHA,
        ONE_MINUS_DST_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
        CONSTANT_COLOUR = VK_BLEND_FACTOR_CONSTANT_COLOR,
        ONE_MINUS_CONSTANT_COLOUR = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
        CONSTANT_ALPHA = VK_BLEND_FACTOR_CONSTANT_ALPHA,
        ONE_MINUS_CONSTANT_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
        SRC_ALPHA_SATURATE = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
        SRC1_COLOUR = VK_BLEND_FACTOR_SRC1_COLOR,
        ONE_MINUS_SRC1_COLOUR = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
        SRC1_ALPHA = VK_BLEND_FACTOR_SRC1_ALPHA,
        ONE_MINUS_SRC1_ALPHA = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA,
    };

    enum class Filter {
        NEAREST = VK_FILTER_NEAREST,
        LINEAR = VK_FILTER_LINEAR,
        CUBIC = VK_FILTER_CUBIC_EXT,
    };

    enum class AddressMode {
        REPEAT = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        MIRRORED_REPEAT = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
        CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        CLAMP_TO_BORDER = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        MIRROR_CLAMP_TO_EDGE = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
    };

    enum class MipmapMode {
        NEAREST = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        LINEAR = VK_SAMPLER_MIPMAP_MODE_LINEAR,
    };

    enum class BorderColour {
        TRANSPARENT_BLACK_FLOAT = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        TRANSPARENT_BLACK_INT = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
        OPAQUE_BLACK_FLOAT = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        OPAQUE_BLACK_INT = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        OPAQUE_WHITE_FLOAT = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
        OPAQUE_WHITE_INT = VK_BORDER_COLOR_INT_OPAQUE_WHITE,
    };

#define ENUM_OPERATOR(enum, op) \
    constexpr enum operator op(enum lhs, enum rhs) noexcept { \
        return static_cast<enum>(static_cast<std::underlying_type<enum>::type>(lhs) op static_cast<std::underlying_type<enum>::type>(rhs)); \
    }

#define ENUM_OPERATOR_EQUALS(enum, op) \
    constexpr enum operator op##=(enum &lhs, enum rhs) noexcept { \
        lhs = static_cast<enum>(static_cast<std::underlying_type<enum>::type>(lhs) op static_cast<std::underlying_type<enum>::type>(rhs)); \
        return lhs; \
    }


    ENUM_OPERATOR(ShaderStage, &)
    ENUM_OPERATOR_EQUALS(ShaderStage, &)
    ENUM_OPERATOR(ShaderStage, |)
    ENUM_OPERATOR_EQUALS(ShaderStage, |)

    ENUM_OPERATOR(PipelineStage, &)
    ENUM_OPERATOR_EQUALS(PipelineStage, &)
    ENUM_OPERATOR(PipelineStage, |)
    ENUM_OPERATOR_EQUALS(PipelineStage, |)

    ENUM_OPERATOR(Access, &)
    ENUM_OPERATOR_EQUALS(Access, &)
    ENUM_OPERATOR(Access, |)
    ENUM_OPERATOR_EQUALS(Access, |)

    ENUM_OPERATOR(ImageUsage, &)
    ENUM_OPERATOR_EQUALS(ImageUsage, &)
    ENUM_OPERATOR(ImageUsage, |)
    ENUM_OPERATOR_EQUALS(ImageUsage, |)

    ENUM_OPERATOR(BufferUsage, &)
    ENUM_OPERATOR_EQUALS(BufferUsage, &)
    ENUM_OPERATOR(BufferUsage, |)
    ENUM_OPERATOR_EQUALS(BufferUsage, |)


    constexpr inline bool isDepthFormat(Format format) {
        switch (format) {
            case Format::D16_UNORM:
            case Format::D16_UNORM_S8_UINT:
            case Format::D24_UNORM_S8_UINT:
            case Format::D32_SFLOAT:
            case Format::D32_SFLOAT_S8_UINT:
                //TODO: check for more
                return true;
            default:
                return false;
        }
        return false;
    }
}

#endif //CANTA_ENUMS_H
