cmake_minimum_required(VERSION 3.11)

# you may need to set following cmake_policies before your **first project() function**
if (POLICY CMP0042)
  cmake_policy (SET CMP0042 NEW)
endif ()

if (POLICY CMP0048)
  cmake_policy (SET CMP0048 NEW)
endif ()

if (POLICY CMP0063)
  cmake_policy (SET CMP0063 NEW)
endif ()


project(example)


# gflags cmake flags

# hide pathname in help message
# set(GFLAGS_HIDE_PATH_IN_HELP "All") # Hide entire pathname
set(GFLAGS_HIDE_PATH_IN_HELP "Basename") # Hide only the basename of pathname

# Use shortest possible double precision in help message
set(GFLAGS_USE_SHORTEST_DOUBLE_PRECISION ON)

# set C++ namespace for gflags. (default "gflags")
# you can use GFLAGS_NAMESLACE::function() in your code.
# set(GFLAGS_NAMESPACE "google")

# add using add_subdirectory
# since gflags library's primary CMakeLists.txt are in our parent directory,
# we need to use some tricks here for it to work
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../ ${CMAKE_CURRENT_BINARY_DIR}/gflags)


# # add using FetchContent
# include(FetchContent)
# FetchContent_Declare(gflags
#         GIT_REPOSITORY          https://github.com/Gavin1937/gflags.git
#         GIT_TAG                 mod
# )
# FetchContent_GetProperties(gflags)
# if(NOT gflags_POPULATED)
#         FetchContent_Populate(gflags)
#         cmake_policy(SET CMP0069 NEW)
#         add_subdirectory(${gflags_SOURCE_DIR} ${gflags_BINARY_DIR})
# endif()

message(STATUS "GFLAGS_HIDE_PATH_IN_HELP: ${GFLAGS_HIDE_PATH_IN_HELP}")
message(STATUS "GFLAGS_NAMESPACE: ${GFLAGS_NAMESPACE}")


set(SRC main.cpp args.cpp args.hpp)

add_executable(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} gflags)
