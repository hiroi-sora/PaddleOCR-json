cmake_minimum_required(VERSION 3.24)
project(ppocr VERSION 1.2.1)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/installed)

include(CTest)
enable_testing()

add_subdirectory(thirdparty)
add_subdirectory(src)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
