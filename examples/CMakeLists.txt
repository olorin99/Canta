add_executable(hello_triangle hello_triangle.cpp)
target_link_libraries(hello_triangle Canta imgui)

add_executable(hello_particles hello_particles.cpp)
target_link_libraries(hello_particles Canta imgui)

add_executable(hello_rays hello_rays.cpp)
target_link_libraries(hello_rays Canta imgui)

compile_shaders(matrix_multiply.slang)
embed_shaders(
        NAME Matrix
        SHADERS ${COMPILED_SHADER_OUTPUTS}
)

add_executable(matrix_multiply matrix_multiply.cpp)
target_link_libraries(matrix_multiply Canta Matrix_SHADERS)

add_executable(hello_sort hello_sort.cpp)
target_link_libraries(hello_sort Canta)

add_executable(prefix_sum prefix_sum.cpp)
target_link_libraries(prefix_sum Canta)