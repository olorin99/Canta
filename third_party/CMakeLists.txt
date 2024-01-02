include(FetchContent)

FetchContent_Declare(
    Ende
    GIT_REPOSITORY https://github.com/olorin99/Ende
    GIT_TAG        9b2983c894f4be92d2cd7b1c0a65c7b860502858
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
    GIT_TAG         1ef8d3ce348daf5d580e27fc68e91628ce42c1f4
    FIND_PACKAGE_ARGS NAMES spdlog
)

FetchContent_MakeAvailable(Ende volk vma spdlog)