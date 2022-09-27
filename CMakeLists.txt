cmake_minimum_required(VERSION 3.5)
project(fastddr-pr-minimal-examples)
set(CMAKE_VERBOSE_MAKEFILE ON)

#################################################
# compiler settings
#################################################

# Default to C11
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(Fast-DDS)
add_subdirectory(src/nested-sequences-bug)
add_subdirectory(src/nested-field-type-registration-bug)
