project(PaddleOCR-json CXX C)
cmake_minimum_required(VERSION 3.14)

set(DEMO_NAME "PaddleOCR-json") 

# 令CMake将最后的可执行文件放在 build/bin 里面
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# 编译相关参数
option(WITH_MKL          "使用MKL或OpenBlas，默认使用MKL。"                                  ON)
option(WITH_GPU          "使用GPU或CPU，默认使用CPU。"                                      OFF)
option(WITH_STATIC_LIB   "编译成static library或shared library，默认编译成static library。"   ON)
option(WITH_TENSORRT     "使用TensorRT，默认关闭。"                                         OFF)

if (UNIX AND NOT APPLE) # Linux
    # 在Linux环境下使用 `WITH_STATIC_LIB=ON` 时无法编译
    # https://github.com/PaddlePaddle/PaddleOCR/issues/10602
    # 这里就直接强行设置成 `OFF` 了
    set(WITH_STATIC_LIB OFF)
endif ()

# 依赖库路径相关参数
SET(PADDLE_LIB "" CACHE PATH "paddle_inference的路径")
SET(OPENCV_DIR "" CACHE PATH "库的路径。Linux下，如果已经安装到系统之中就不用指定了。")
SET(CUDA_LIB "" CACHE PATH "库的路径")
SET(CUDNN_LIB "" CACHE PATH "库的路径")
SET(TENSORRT_DIR "" CACHE PATH "使用TensorRT编译并设置其路径")

# 功能相关参数
option(ENABLE_CLIPBOARD         "启用剪贴板功能。默认关闭。"        OFF)
option(ENABLE_REMOTE_EXIT       "启用远程关停服务器命令。默认开启。"  ON)
option(ENABLE_JSON_IMAGE_PATH   "启用json命令image_path。默认开启。" ON)


# compiler flags

# 检测是否运行在WSL下，如果是则设置compiler flag：-DUNDER_WSL
if (UNIX AND NOT APPLE) # Linux
    execute_process (
        COMMAND "uname" "-a"
        OUTPUT_VARIABLE UNAME_RESULT
    )
    string(STRIP "${UNAME_RESULT}" UNAME_RESULT)
    if (UNAME_RESULT MATCHES ".*WSL.*")
        message(STATUS "Compiling under WSL")
        add_definitions(-DUNDER_WSL)
    endif()
endif ()

# 在Windows下如果启用剪贴板，设置一个compiler flag：-DENABLE_CLIPBOARD
if (ENABLE_CLIPBOARD AND WIN32)
    add_definitions(-DENABLE_CLIPBOARD)
elseif () # 在其他系统上禁用ENABLE_CLIPBOARD
    set(ENABLE_CLIPBOARD OFF)
endif()

# 启用远程关停服务器命令，设置compiler flag：-DENABLE_REMOTE_EXIT
if (ENABLE_REMOTE_EXIT)
    add_definitions(-DENABLE_REMOTE_EXIT)
endif()

# 启用json命令image_path，设置compiler flag：-DENABLE_JSON_IMAGE_PATH
if (ENABLE_JSON_IMAGE_PATH)
    add_definitions(-DENABLE_JSON_IMAGE_PATH)
endif()

macro(safe_set_static_flag)
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${flag_var} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var)
endmacro()

if (WITH_MKL)
    ADD_DEFINITIONS(-DUSE_MKL)
endif()

if(NOT DEFINED PADDLE_LIB)
    message(FATAL_ERROR "please set PADDLE_LIB with -DPADDLE_LIB=/path/paddle/lib")
endif()

if(NOT DEFINED OPENCV_DIR)
    message(FATAL_ERROR "please set OPENCV_DIR with -DOPENCV_DIR=/path/opencv")
endif()


# 输出功能设置
message(STATUS "Features:")
message(STATUS "    ENABLE_CLIPBOARD: ${ENABLE_CLIPBOARD}")
message(STATUS "    ENABLE_REMOTE_EXIT: ${ENABLE_REMOTE_EXIT}")
message(STATUS "    ENABLE_JSON_IMAGE_PATH: ${ENABLE_JSON_IMAGE_PATH}")


# 加载依赖

# OpenCV
# 尝试从人工提供的OpenCV路径寻找OpenCV
# 请提供一个路径包含 "OpenCVConfig.cmake" 或 "opencv-config.cmake"这两个文件

# 由CMake参数提供（-DOPENCV_DIR）
if(DEFINED OPENCV_DIR AND NOT "${OPENCV_DIR}" STREQUAL "")
    message(STATUS "Setting OPENCV_DIR: ${OPENCV_DIR}")
    find_package(OpenCV PATHS ${OPENCV_DIR} NO_DEFAULT_PATH)
elseif(DEFINED OpenCV_DIR AND NOT "${OpenCV_DIR}" STREQUAL "")
    message(STATUS "Setting OPENCV_DIR: ${OpenCV_DIR}")
    find_package(OpenCV PATHS ${OpenCV_DIR} NO_DEFAULT_PATH)
# 由环境变量提供 OPENCV_DIR 或 OpenCV_DIR 提供
elseif(NOT "$ENV{OPENCV_DIR}" STREQUAL "")
    message(STATUS "Setting OPENCV_DIR: $ENV{OPENCV_DIR}")
    find_package(OpenCV PATHS $ENV{OPENCV_DIR} NO_DEFAULT_PATH)
elseif(NOT "$ENV{OpenCV_DIR}" STREQUAL "")
    message(STATUS "Setting OPENCV_DIR: $ENV{OpenCV_DIR}")
    find_package(OpenCV PATHS $ENV{OpenCV_DIR} NO_DEFAULT_PATH)
# 没有提供OpenCV路径，尝试让CMake自己寻找
else()
    message("last opencv")
    find_package(OpenCV REQUIRED)
endif()
include_directories(${OpenCV_INCLUDE_DIRS})


# PaddleOCR
include_directories("${PADDLE_LIB}/paddle/include")
link_directories("${PADDLE_LIB}/paddle/lib")


if (WIN32)
    add_definitions("/DGOOGLE_GLOG_DLL_DECL=")
    if(WITH_MKL)
        set(FLAG_OPENMP "/openmp")
    endif()
    set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} /bigobj /MTd ${FLAG_OPENMP}")
    set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} /bigobj /MT ${FLAG_OPENMP}")
    set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} /bigobj /MTd ${FLAG_OPENMP}")
    set(CMAKE_CXX_FLAGS_RELEASE   "${CMAKE_CXX_FLAGS_RELEASE} /bigobj /MT ${FLAG_OPENMP}")
    if (WITH_STATIC_LIB)
        safe_set_static_flag()
        add_definitions(-DSTATIC_LIB)
    endif()
    message(STATUS "cmake c debug flags:" ${CMAKE_C_FLAGS_DEBUG})
    message(STATUS "cmake c release flags:" ${CMAKE_C_FLAGS_RELEASE})
    message(STATUS "cmake cxx debug flags:" ${CMAKE_CXX_FLAGS_DEBUG})
    message(STATUS "cmake cxx release flags:" ${CMAKE_CXX_FLAGS_RELEASE})
else()
    if(WITH_MKL)
        set(FLAG_OPENMP "-fopenmp")
    endif()
    # 在Linux环境下使用 `-g -o3` 无法编译
    # https://github.com/PaddlePaddle/PaddleOCR/issues/4654#issuecomment-977398981
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -o3 ${FLAG_OPENMP} -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG_OPENMP} -std=c++11")
    set(CMAKE_STATIC_LIBRARY_PREFIX "")
    message(STATUS "cmake cxx flags:" ${CMAKE_CXX_FLAGS})
endif()

if (WITH_GPU)
    if (NOT DEFINED CUDA_LIB OR ${CUDA_LIB} STREQUAL "")
        message(FATAL_ERROR "please set CUDA_LIB with -DCUDA_LIB=/path/cuda-8.0/lib64")
    endif()
    if (NOT WIN32)
        if (NOT DEFINED CUDNN_LIB)
            message(FATAL_ERROR "please set CUDNN_LIB with -DCUDNN_LIB=/path/cudnn_v7.4/cuda/lib64")
        endif()
    endif(NOT WIN32)
endif()

include_directories("${PADDLE_LIB}/third_party/install/protobuf/include")
include_directories("${PADDLE_LIB}/third_party/install/glog/include")
include_directories("${PADDLE_LIB}/third_party/install/gflags/include")
include_directories("${PADDLE_LIB}/third_party/install/xxhash/include")
include_directories("${PADDLE_LIB}/third_party/install/zlib/include")
include_directories("${PADDLE_LIB}/third_party/install/onnxruntime/include")
include_directories("${PADDLE_LIB}/third_party/install/paddle2onnx/include")
include_directories("${PADDLE_LIB}/third_party/boost")
include_directories("${PADDLE_LIB}/third_party/eigen3")

include_directories("${CMAKE_SOURCE_DIR}/")

if (NOT WIN32)
    if (WITH_TENSORRT AND WITH_GPU)
        include_directories("${TENSORRT_DIR}/include")
        link_directories("${TENSORRT_DIR}/lib")
    endif()
endif(NOT WIN32)

link_directories("${PADDLE_LIB}/third_party/install/zlib/lib")

link_directories("${PADDLE_LIB}/third_party/install/protobuf/lib")
link_directories("${PADDLE_LIB}/third_party/install/glog/lib")
link_directories("${PADDLE_LIB}/third_party/install/gflags/lib")
link_directories("${PADDLE_LIB}/third_party/install/xxhash/lib")
link_directories("${PADDLE_LIB}/third_party/install/onnxruntime/lib")
link_directories("${PADDLE_LIB}/third_party/install/paddle2onnx/lib")
link_directories("${PADDLE_LIB}/paddle/lib")


if(WITH_MKL)
    include_directories("${PADDLE_LIB}/third_party/install/mklml/include")
    if (WIN32)
        set(MATH_LIB ${PADDLE_LIB}/third_party/install/mklml/lib/mklml.lib
                    ${PADDLE_LIB}/third_party/install/mklml/lib/libiomp5md.lib)
    else ()
        set(MATH_LIB ${PADDLE_LIB}/third_party/install/mklml/lib/libmklml_intel${CMAKE_SHARED_LIBRARY_SUFFIX}
                    ${PADDLE_LIB}/third_party/install/mklml/lib/libiomp5${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif ()
    set(MKLDNN_PATH "${PADDLE_LIB}/third_party/install/mkldnn")
    if(EXISTS ${MKLDNN_PATH})
        include_directories("${MKLDNN_PATH}/include")
        if (WIN32)
            set(MKLDNN_LIB ${MKLDNN_PATH}/lib/mkldnn.lib)
        else ()
            set(MKLDNN_LIB ${MKLDNN_PATH}/lib/libmkldnn.so.0)
        endif ()
    endif()
    else()
    if (WIN32)
        set(MATH_LIB ${PADDLE_LIB}/third_party/install/openblas/lib/openblas${CMAKE_STATIC_LIBRARY_SUFFIX})
    else ()
        set(MATH_LIB ${PADDLE_LIB}/third_party/install/openblas/lib/libopenblas${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif ()
endif()

# Note: libpaddle_inference_api.so/a must put before libpaddle_inference.so/a
if(WITH_STATIC_LIB)
    if(WIN32)
        set(DEPS
            ${PADDLE_LIB}/paddle/lib/paddle_inference${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        set(DEPS
            ${PADDLE_LIB}/paddle/lib/libpaddle_inference${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
else()
    if(WIN32)
        set(DEPS
            ${PADDLE_LIB}/paddle/lib/paddle_inference${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
        set(DEPS
            ${PADDLE_LIB}/paddle/lib/libpaddle_inference${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
endif(WITH_STATIC_LIB)

if (NOT WIN32)
    set(DEPS ${DEPS}
        ${MATH_LIB} ${MKLDNN_LIB}
        glog gflags protobuf z xxhash
        )
    if(EXISTS "${PADDLE_LIB}/third_party/install/snappystream/lib")
        set(DEPS ${DEPS} snappystream)
    endif()
    if (EXISTS "${PADDLE_LIB}/third_party/install/snappy/lib")
        set(DEPS ${DEPS} snappy)
    endif()
else()
    set(DEPS ${DEPS}
        ${MATH_LIB} ${MKLDNN_LIB}
        glog gflags_static libprotobuf xxhash)
    set(DEPS ${DEPS} libcmt shlwapi)
    if (EXISTS "${PADDLE_LIB}/third_party/install/snappy/lib")
        set(DEPS ${DEPS} snappy)
    endif()
    if(EXISTS "${PADDLE_LIB}/third_party/install/snappystream/lib")
        set(DEPS ${DEPS} snappystream)
    endif()
endif(NOT WIN32)


if(WITH_GPU)
    if(NOT WIN32)
        if (WITH_TENSORRT)
            set(DEPS ${DEPS} ${TENSORRT_DIR}/lib/libnvinfer${CMAKE_SHARED_LIBRARY_SUFFIX})
            set(DEPS ${DEPS} ${TENSORRT_DIR}/lib/libnvinfer_plugin${CMAKE_SHARED_LIBRARY_SUFFIX})
        endif()
        set(DEPS ${DEPS} ${CUDA_LIB}/libcudart${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(DEPS ${DEPS} ${CUDNN_LIB}/libcudnn${CMAKE_SHARED_LIBRARY_SUFFIX})
    else()
        set(DEPS ${DEPS} ${CUDA_LIB}/cudart${CMAKE_STATIC_LIBRARY_SUFFIX} )
        set(DEPS ${DEPS} ${CUDA_LIB}/cublas${CMAKE_STATIC_LIBRARY_SUFFIX} )
        set(DEPS ${DEPS} ${CUDNN_LIB}/cudnn${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
endif()


if (NOT WIN32)
    set(EXTERNAL_LIB "-ldl -lrt -lgomp -lz -lm -lpthread")
    set(DEPS ${DEPS} ${EXTERNAL_LIB})
endif()

set(DEPS ${DEPS}
    opencv_core
    opencv_imgcodecs
    opencv_imgproc
)

include(FetchContent)
include(external-cmake/auto-log.cmake)
include_directories(${FETCHCONTENT_BASE_DIR}/extern_autolog-src)

AUX_SOURCE_DIRECTORY(./src SRCS)
add_executable(${DEMO_NAME} ${SRCS})
target_link_libraries(${DEMO_NAME} ${DEPS})


# 把 PADDLE_LIB 文件夹下的所有共享库找出
file(GLOB_RECURSE PADDLE_LIB_FILES "${PADDLE_LIB}/*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
# 然后全部复制到TARGET的输出文件夹里
foreach(ITEM ${PADDLE_LIB_FILES})
    add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ITEM}
        "$<TARGET_FILE_DIR:${DEMO_NAME}>"
    )
endforeach()


# CMake TARGET安装路径设置
# 参考OpenCV：
# https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT.html
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # 本地编译
    if(NOT CMAKE_TOOLCHAIN_FILE)
        if(WIN32)
            set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "安装路径" FORCE)
        else()
            set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "安装路径" FORCE)
        endif()
    
    # 其他跨平台编译
    else()
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "安装路径" FORCE)
    endif()
endif()

# 安装TARGET
install(
    TARGETS ${DEMO_NAME}
    CONFIGURATIONS Debug Release
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
)
# 其他安装
if (WIN32)
    # 安装共享库
    install(
        DIRECTORY "$<TARGET_FILE_DIR:${DEMO_NAME}>/"
        DESTINATION "bin"
        FILES_MATCHING
        PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
        PATTERN "${DEMO_NAME}*" EXCLUDE
    )
else ()
    # 安装共享库
    install(
        DIRECTORY "$<TARGET_FILE_DIR:${DEMO_NAME}>/"
        DESTINATION "lib"
        FILES_MATCHING
        PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
        PATTERN "${DEMO_NAME}*" EXCLUDE
    )
    # 安装启动脚本和安装脚本
    install(
        FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/linux_dist_tools/run.sh"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/linux_dist_tools/install_env.sh"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        DESTINATION "."
    )
endif ()
