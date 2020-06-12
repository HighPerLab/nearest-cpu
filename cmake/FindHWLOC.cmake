find_path(
    HWLOC_INCLUDE_DIR
    NAMES hwloc.h
    PATHS ENV HWLOCROOT
    PATH_SUFFIXES "include"
)

find_path(HWLOC_INCLUDE_DIR NAMES hwloc.h)

find_library(
    HWLOC_LIBRARY
    NAMES hwloc.a hwloc libhwloc
    PATHS ENV HWLOCROOT
    PATH_SUFFIXES "lib"
)

find_library(HWLOC_LIBRARY NAMES hwloc.a hwloc libhwloc)

set(HWLOC_LIBRARIES ${HWLOC_LIBRARY})
set(HWLOC_INCLUDE_DIRS ${HWLOC_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(HWLOC DEFAULT_MSG HWLOC_LIBRARY HWLOC_INCLUDE_DIR)
