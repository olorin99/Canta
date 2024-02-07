FetchContent_Declare(
        imgui
        GIT_REPOSITORY  https://github.com/ocornut/imgui
        GIT_TAG         763100b38580dc661e190452b35bf64efd01147e
)
FetchContent_MakeAvailable(imgui)


set(IMGUI_PATH ${imgui_SOURCE_DIR})
set(IMGUI_SOURCES ${IMGUI_PATH}/imgui.cpp ${IMGUI_PATH}/imgui_demo.cpp ${IMGUI_PATH}/imgui_tables.cpp ${IMGUI_PATH}/imgui_widgets.cpp ${IMGUI_PATH}/imgui_draw.cpp ${IMGUI_PATH}/backends/imgui_impl_vulkan.cpp ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

add_executable(hello_triangle hello_triangle.cpp)
target_link_libraries(hello_triangle Canta imgui)

add_executable(hello_particles hello_particles.cpp)
target_link_libraries(hello_particles Canta imgui)