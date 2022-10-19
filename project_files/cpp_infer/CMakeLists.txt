cmake_minimum_required(VERSION 3.24)
project(PaddleOCR_json VERSION 1.2.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/installed)

include(CTest)
enable_testing()

add_subdirectory(thirdparty)
add_subdirectory(src)

set(CPACK_GENERATOR ZIP)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
