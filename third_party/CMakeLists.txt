include(FetchContent)

FetchContent_Declare(
    Ende
    GIT_REPOSITORY https://github.com/olorin99/Ende
    GIT_TAG        a0a2dc20e581e13f55a9acfce6b6b099d989942c
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
    imgui
    GIT_REPOSITORY  https://github.com/ocornut/imgui
    GIT_TAG         "docking"
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
    GIT_TAG         f45ebce73b3631fdfb8205e2ba700b726ff0c34f
)

FetchContent_Declare(
    spirv_cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross
    GIT_TAG        f349c91274b91c1a7c173f2df70ec53080076191
)
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF)

FetchContent_Declare(
    spirv-headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers
    GIT_TAG        1c6bb2743599e6eb6f37b2969acc0aef812e32e3
)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    spirv-tools
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools
    GIT_TAG        f0cc85efdbbe3a46eae90e0f915dc1509836d0fc
)
set(SPIRV_SKIP_TESTS OFF CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_WERROR OFF CACHE BOOL "" FORCE)
set(SPIRV_WARN_EVERYTHING OFF CACHE BOOL "" FORCE)
set(ENABLE_EXCEPTIONS_ON_MSVC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang
    GIT_TAG        db4d6f85afb8cf6aa404a141855f556d172c1ed2
)
set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)
set(BUILD_WERROR OFF CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_RTTI OFF CACHE BOOL "" FORCE)
set(ENABLE_EXCEPTIONS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    shaderc
    GIT_REPOSITORY https://github.com/google/shaderc
    GIT_TAG        aaa44b544909600381e1a180074ed7f544e48410
)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_ENABLE_WERROR_COMPILE OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)


FetchContent_MakeAvailable(Ende volk vma imgui spdlog tsl_map spirv_cross spirv-headers spirv-tools glslang shaderc)

set(IMGUI_PATH ${imgui_SOURCE_DIR})
set(IMGUI_SOURCES ${IMGUI_PATH}/imgui.cpp ${IMGUI_PATH}/imgui_demo.cpp ${IMGUI_PATH}/imgui_tables.cpp ${IMGUI_PATH}/imgui_widgets.cpp ${IMGUI_PATH}/imgui_draw.cpp ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})