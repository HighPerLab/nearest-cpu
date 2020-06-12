cmake_minimum_required(VERSION 3.17)
project (nearest-gpu LANGUAGES C)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package (HWLOC REQUIRED)
find_package (CUDAToolkit REQUIRED)

add_executable(nearest-gpu nearest-gpu.c)
target_include_directories (nearest-gpu PRIVATE ${CUDAToolkit_INCLUDE_DIRS} ${HWLOC_INCLUDE_DIRS})
target_link_libraries (nearest-gpu PRIVATE ${HWLOC_LIBRARIES} CUDA::cudart)
