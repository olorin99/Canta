cmake_minimum_required(VERSION 3.20)
project(Canta)

set(CMAKE_CXX_STANDARD 23)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

option(CANTA_BUILD_EXAMPLES "enable examples" ON)
option(CANTA_ENABLE_IMGUI "enable imgui" ON)
option(CANTA_BUILD_TESTS "build tests" ON)
option(CANTA_USE_SLANG "use slang shaders" ON)

add_compile_definitions(CANTA_SRC_DIR="${CMAKE_SOURCE_DIR}")

find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)
find_package(slang REQUIRED)

function(embed_shaders)
    cmake_parse_arguments(
            PARSED_ARGS
            ""
            "NAME"
            "SHADERS"
            ${ARGN}
    )
    add_custom_command(
            OUTPUT
            "${CMAKE_CURRENT_BINARY_DIR}/include/embeded_shaders_${PARSED_ARGS_NAME}.h"
            COMMAND python ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_files.py ${PARSED_ARGS_NAME} ${CMAKE_CURRENT_BINARY_DIR}/include/embeded_shaders_${PARSED_ARGS_NAME}.h ${PARSED_ARGS_SHADERS}
            DEPENDS
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_files.py
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    add_custom_target(embeded_files_${PARSED_ARGS_NAME}
            DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/include/embeded_shaders_${PARSED_ARGS_NAME}.h"
    )
    add_library(${PARSED_ARGS_NAME}_SHADERS INTERFACE)
    target_include_directories(${PARSED_ARGS_NAME}_SHADERS
            INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    add_dependencies(${PARSED_ARGS_NAME}_SHADERS embeded_files_${PARSED_ARGS_NAME})
endfunction()

embed_shaders(
        NAME Canta
        SHADERS "canta.slang:${CMAKE_CURRENT_SOURCE_DIR}/src/canta.slang" "examples/particles.slang:${CMAKE_CURRENT_SOURCE_DIR}/examples/particles.slang"
)

add_library(Canta
        src/Device.cpp
        include/Canta/Device.h
        include/Canta/Enums.h
        src/Swapchain.cpp
        include/Canta/Swapchain.h
        src/Semaphore.cpp
        include/Canta/Semaphore.h
        include/Canta/Window.h
        src/SDLWindow.cpp
        include/Canta/SDLWindow.h
        src/CommandPool.cpp
        include/Canta/CommandPool.h
        src/CommandBuffer.cpp
        include/Canta/CommandBuffer.h
        src/util.cpp
        include/Canta/util.h
        include/Canta/ResourceList.h
        src/ShaderModule.cpp
        include/Canta/ShaderModule.h
        src/ShaderInterface.cpp
        include/Canta/ShaderInterface.h
        src/Pipeline.cpp
        include/Canta/Pipeline.h
        src/Image.cpp
        include/Canta/Image.h
        src/Buffer.cpp
        include/Canta/Buffer.h
        src/Sampler.cpp
        include/Canta/Sampler.h
        src/Timer.cpp
        include/Canta/Timer.h
        src/Queue.cpp
        include/Canta/Queue.h
        src/PipelineManager.cpp
        include/Canta/PipelineManager.h
        src/RenderGraph.cpp
        include/Canta/RenderGraph.h
        src/ImGuiContext.cpp
        include/Canta/ImGuiContext.h
        src/PipelineStatistics.cpp
        include/Canta/PipelineStatistics.h
        src/UploadBuffer.cpp
        include/Canta/UploadBuffer.h
)

add_subdirectory(third_party)

target_include_directories(Canta
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    third_party
    PRIVATE
    src
    third_party
    slang::slang
)

target_link_libraries(Canta
    Ende
    volk
    GPUOpen::VulkanMemoryAllocator
    spdlog
    ${SDL2_LIBRARIES}
    spirv-cross-cpp
    shaderc_combined
    tsl::robin_map
    Threads::Threads
    slang::slang
    Canta_SHADERS
)



if (CANTA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (CANTA_ENABLE_IMGUI)
    target_link_libraries(Canta imgui)
endif ()

if (CANTA_BUILD_TESTS)
    add_subdirectory(tests)
endif ()