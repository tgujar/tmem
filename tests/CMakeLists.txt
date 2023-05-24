cmake_minimum_required(VERSION 3.5)
project(TestSuite)

set(CHECK_INCLUDE_DIRS "/opt/homebrew/Cellar/check/0.15.2/include")
set(CHECK_LIBRARIES "/opt/homebrew/Cellar/check/0.15.2/lib/libcheck.dylib")

# # Add the Check library
# find_package(PkgConfig REQUIRED)
# pkg_check_modules(CHECK REQUIRED check)

# Create the test suite target
add_executable(test_suite test_tmalloc.c)

# Include the Check library's header files directory
target_include_directories(test_suite PRIVATE ${CHECK_INCLUDE_DIRS})

# Link the library and Check with the test suite
target_link_libraries(test_suite PRIVATE tmem ${CHECK_LIBRARIES})

# Optionally, you can enable additional warnings and optimizations
# for the release build type
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -O2")

# Set the output directory for the test suite binary
set_target_properties(test_suite
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin"
)
