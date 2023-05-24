cmake_minimum_required(VERSION 3.5)
project(tmem)

# Custom cmake modules
# set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(BUILD_SHARED_LIBS OFF)

# Add your source files
set(SOURCES
    tmalloc.c
    # Add more source files if needed
)

# Create the library target
add_library(tmem ${SOURCES})

target_include_directories(tmem PUBLIC include)
set_target_properties(tmem PROPERTIES PUBLIC_HEADER include/tmalloc.h)

# Optionally, you can enable additional warnings and optimizations
# for the release build type
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -O2")


install(TARGETS tmem
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include
)

enable_testing()

add_subdirectory(tests)