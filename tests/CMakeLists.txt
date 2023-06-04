include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src ../include)

find_package(Check REQUIRED)
include_directories(${CHECK_INCLUDE_DIRS})
link_directories(${CHECK_LIBRARY_DIRS})

set(TEST_SOURCES
  test_tmalloc.c
)

add_executable(check_tmem ${TEST_SOURCES})
target_link_libraries(check_tmem tmem ${CHECK_LIBRARIES})