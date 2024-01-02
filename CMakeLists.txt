cmake_minimum_required(VERSION 3.20)
project(Canta)

set(CMAKE_CXX_STANDARD 23)

add_library(Canta
        src/Device.cpp
        include/Canta/Device.h
        include/Canta/Enums.h)

add_subdirectory(third_party)

target_include_directories(Canta
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
    third_party
    PRIVATE
    src
    third_party
)

target_link_libraries(Canta
    Ende
    volk
    GPUOpen::VulkanMemoryAllocator
    spdlog::spdlog
)

add_subdirectory(examples)