cmake_minimum_required(VERSION 3.20)
project(Canta)

set(CMAKE_CXX_STANDARD 23)

#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

option(CANTA_BUILD_EXAMPLES "enable examples" ON)
option(CANTA_ENABLE_IMGUI "enable imgui" ON)
option(CANTA_BUILD_TESTS "build tests" ON)
option(CANTA_ENABLE_RENDERDOC "enable renderdoc" ON)

if (${CANTA_ENABLE_RENDERDOC})
    add_compile_definitions(CANTA_RENDERDOC="${CANTA_ENABLE_RENDERDOC}")
endif ()

add_compile_definitions(CANTA_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(third_party)

function(compile_shaders)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include")
    set(SHADER_OUTPUTS "")
    foreach (SHADER ${ARGV})
        set(SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
        message("Path: ${SHADER_PATH}")
        get_filename_component(file_name ${SHADER_PATH} NAME)
        get_filename_component(base_name ${SHADER_PATH} NAME_WLE)

        set(OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/include/${base_name}.spv")
        list(APPEND SHADER_OUTPUTS ${OUTPUT_FILE})

        set(SLANG_BINARY "${SLANG_BINARY_DIR}/slang-2025.24.3-linux-x86_64/bin/slangc")

        add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                DEPENDS slang ${SHADER_PATH}
                COMMAND ${SLANG_BINARY} -I${CMAKE_SOURCE_DIR}/src ${SHADER_PATH} -o ${OUTPUT_FILE}
        )
    endforeach ()
    set(COMPILED_SHADER_OUTPUTS ${SHADER_OUTPUTS} PARENT_SCOPE)

endfunction()

function(embed_shaders)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include")
    cmake_parse_arguments(
            PARSED_ARGS
            ""
            "NAME"
            "SHADERS"
            ${ARGN}
    )
    set(OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/include/embedded_shaders_${PARSED_ARGS_NAME}.h")
    add_custom_command(
            OUTPUT
            ${OUTPUT_HEADER}
            COMMAND python ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_files.py ${PARSED_ARGS_NAME} ${OUTPUT_HEADER} ${PARSED_ARGS_SHADERS}
            DEPENDS
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embed_files.py ${PARSED_ARGS_SHADERS}
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    add_custom_target(embedded_files_${PARSED_ARGS_NAME}
            DEPENDS ${OUTPUT_HEADER}
    )
    add_library(${PARSED_ARGS_NAME}_SHADERS INTERFACE)
    target_include_directories(${PARSED_ARGS_NAME}_SHADERS
            INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/include"
    )
    add_dependencies(${PARSED_ARGS_NAME}_SHADERS embedded_files_${PARSED_ARGS_NAME})
endfunction()

compile_shaders(
        src/util/single_sort.slang
        src/util/multi_sort.slang
        src/util/multi_sort_histograms.slang
)

embed_shaders(
        NAME Canta
        SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/src/canta.slang" "${CMAKE_CURRENT_SOURCE_DIR}/src/util/single_sort.slang" "${CMAKE_CURRENT_SOURCE_DIR}/examples/particles.slang" ${COMPILED_SHADER_OUTPUTS}
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
        include/Canta/Camera.h
        src/Camera.cpp
        src/debug/RenderGraphDebugger.cpp
        include/Canta/debug/RenderGraphDebugger.h
        src/debug/PipelineManagerDebugger.cpp
        include/Canta/debug/PipelineManagerDebugger.h
        src/debug/CommandQueueDebugger.cpp
        include/Canta/debug/CommandQueueDebugger.h
        src/util/sort.cpp
        include/Canta/util/sort.h
        include/Canta/util/KernelHelper.h
)

target_include_directories(Canta
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${CMAKE_CURRENT_LIST_DIR}/third_party
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/third_party
    slang
)

target_link_libraries(Canta
    Ende
    volk
    GPUOpen::VulkanMemoryAllocator
    spdlog
    ${SDL2_LIBRARIES}
    spirv-cross-cpp
    tsl::robin_map
    Threads::Threads
    slang
    Canta_SHADERS
)


if (CANTA_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (CANTA_ENABLE_IMGUI)
    target_link_libraries(Canta imgui)
    target_link_libraries(Canta imnodes)
endif ()

if (CANTA_BUILD_TESTS)
    add_subdirectory(tests)
endif ()