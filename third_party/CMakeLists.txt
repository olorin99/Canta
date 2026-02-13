include(FetchContent)

set(ENDE_BUILD_EXAMPLES OFF CACHE BOOL "")
FetchContent_Declare(
    Ende
    GIT_REPOSITORY https://git.melamar.xyz/olorin99/Ende
    GIT_TAG        "master"
)

FetchContent_Declare(
    volk
    GIT_REPOSITORY  https://github.com/zeux/volk
    GIT_TAG         443b10b6d4cd4c889565e8478b3e512ee02ea898
)

FetchContent_Declare(
    vma
    GIT_REPOSITORY  https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG         5e43c795daf43dd09398d8307212e85025215052
)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog
    GIT_TAG         "v1.x"
    FIND_PACKAGE_ARGS NAMES spdlog
)

FetchContent_Declare(
    tsl_map
    GIT_REPOSITORY  https://github.com/Tessil/robin-map
    GIT_TAG         4ec1bf19c6a96125ea22062f38c2cf5b958e448e
)

FetchContent_Declare(
    spirv_cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross
    GIT_TAG        9daa7fe0a36dbe5c5b810d74d2cc43b202407d57
)
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF)

option(SLANG_GIT_REPOSITORY "Repository where we search for the shader slang library." https://github.com/shader-slang/slang.git)
if(NOT SLANG_GIT_REPOSITORY)
    set(SLANG_GIT_REPOSITORY "https://github.com/shader-slang/slang.git")
endif()

option(SLANG_GIT_TAG "Version of the slang library we are fetching." v2025.24.3)
if(NOT SLANG_GIT_TAG)
    set(SLANG_GIT_TAG "v2025.24.3")
endif()

set(IMGUI_PATH imgui)

set(SLANG_EMBED_CORE_MODULE ON CACHE BOOL "" FORCE)
set(SLANG_EMBED_CORE_MODULE_SOURCE ON CACHE BOOL "" FORCE)
set(SLANG_LIB_TYPE STATIC)
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_GFX OFF CACHE BOOL "" FORCE)
set(SLANG_ENABLE_SLANG_RHI OFF CACHE BOOL "" FORCE)

FetchContent_Declare(slang
        GIT_REPOSITORY "${SLANG_GIT_REPOSITORY}"
        GIT_TAG "${SLANG_GIT_TAG}"
)

FetchContent_MakeAvailable(Ende volk vma spdlog tsl_map spirv_cross slang)

set(SLANG_BINARY_DIR ${slang_BINARY_DIR} PARENT_SCOPE)

set(IMGUI_SOURCES ${IMGUI_PATH}/imgui.cpp ${IMGUI_PATH}/imgui_demo.cpp ${IMGUI_PATH}/imgui_tables.cpp ${IMGUI_PATH}/imgui_widgets.cpp ${IMGUI_PATH}/imgui_draw.cpp ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_PATH})

set(IMGUI_NODES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party)
set(IMGUI_NODES_SOURCES
        imnodes.cpp
        imnodes.h
        imnodes_internal.h)
add_library(imnodes ${IMGUI_NODES_SOURCES})
target_link_libraries(imnodes imgui)