# cmake_minimum_required(VERSION 3.15...3.30)
cmake_minimum_required(VERSION 3.14)
# 这里本来想给cmake_minimum_required设置一下版本范围，
# 然后发现如果将cmake最低版本改成3.15时，Windows MSVC下会出现
# 大量的链接错误，基本都是一些libc底层函数的unresolved external symbol
# 和symbol redefinition错误。具体原因未知，即使把cmake 3.15的新policies
# 给禁用了也没用。不知道是3.15的什么改动造成的。不过现在用3.14也没有问题。
# 
# cmake version range:
# https://cmake.org/cmake/help/latest/command/cmake_minimum_required.html
# https://github.com/jbeder/yaml-cpp/pull/1211#issuecomment-1681352550


project(PaddleOCR-json CXX C)

# 消除CMake Policy CMP0048警告
# 让CMake以新的模式来处理policy CMP0048
# https://cmake.org/cmake/help/latest/policy/CMP0048.html
if (POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif ()


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

# CMake功能相关参数
option(INSTALL_WITH_TOOLS       "CMake安装时附带工具文件。默认开启。"      ON)


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
# 输出CMake功能设置
message(STATUS "CMake Features:")
message(STATUS "    INSTALL_WITH_TOOLS: ${INSTALL_WITH_TOOLS}")


# 加载依赖

# OpenCV
# 尝试从人工提供的OpenCV路径寻找OpenCV
# 请提供一个路径包含 "OpenCVConfig.cmake" 或 "opencv-config.cmake"这两个文件
# 或者提供 OpenCV 编译后的安装目录

set(OPENCV_COMPONENTS_TO_LOAD
    core
    imgproc
    imgcodecs
)

# 由CMake参数提供（-DOPENCV_DIR）
if(DEFINED OPENCV_DIR AND NOT "${OPENCV_DIR}" STREQUAL "")
    message(STATUS "Setting OPENCV_DIR: ${OPENCV_DIR}")
    find_package(OpenCV REQUIRED PATHS ${OPENCV_DIR} COMPONENTS ${OPENCV_COMPONENTS_TO_LOAD} NO_DEFAULT_PATH)
    set(CUSTOM_OPENCV_DIR ${OPENCV_DIR})
# 没有提供OpenCV路径，尝试让CMake自己寻找
else()
    find_package(OpenCV REQUIRED COMPONENTS ${OPENCV_COMPONENTS_TO_LOAD})
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

# 加载gflags

# 将gflags编译成静态库
set(GFLAGS_BUILD_STATIC_LIBS ON)

# 隐藏帮助信息里的路径名，并且只保留最后的文件名
set(GFLAGS_HIDE_PATH_IN_HELP "Basename")

# 使用尽可能短的浮点数精度
set(GFLAGS_USE_SHORTEST_DOUBLE_PRECISION ON)

# 设置gflags的C++ namespace为 google
set(GFLAGS_NAMESPACE "google")

# 添加gflags cmake项目
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party/gflags")
include_directories(${gflags_BINARY_DIR}/include)


# 加载paddle_inference里面的其他第三方库
include_directories("${PADDLE_LIB}/third_party/install/protobuf/include")
include_directories("${PADDLE_LIB}/third_party/install/glog/include")
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
        # 添加库搜索目录，寻找 third_party/install/onednn/lib/libdnnl.so.3
        link_directories("${PADDLE_LIB}/third_party/install/onednn/lib")
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
            ${PADDLE_LIB}/paddle/lib/paddle_inference${CMAKE_STATIC_LIBRARY_SUFFIX})
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
else() # WIN32
    set(DEPS ${DEPS}
        ${MATH_LIB} ${MKLDNN_LIB}
        glog gflags libprotobuf xxhash)
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

# 在find_package时已经指定了需要加载的模块，
# 这里直接用OpenCV_LIBS就行了
set(DEPS ${DEPS} ${OpenCV_LIBS})

include(FetchContent)
include(external-cmake/auto-log.cmake)
include_directories(${FETCHCONTENT_BASE_DIR}/extern_autolog-src)

AUX_SOURCE_DIRECTORY(./src SRCS)
add_executable(${DEMO_NAME} ${SRCS})
target_link_libraries(${DEMO_NAME} ${DEPS})


# 设置需要安装的共享库

# PaddleOCR，直接找出其路径下所有共享库即可
message(STATUS "Collecting PaddleOCR shared libraries")
file(GLOB_RECURSE PADDLE_LIB_FILES "${PADDLE_LIB}/**/*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
foreach(ITEM ${PADDLE_LIB_FILES})
    list(APPEND LIBS_TO_INSTALL ${ITEM})
endforeach()

# OpenCV
include(cmake/opencv-install-utils.cmake)
if(DEFINED CUSTOM_OPENCV_DIR AND NOT "${CUSTOM_OPENCV_DIR}" STREQUAL "")
    message(STATUS "Collecting custom OpenCV shared libraries")
    set(OPENCV_MISS_LIBS FALSE)
    
    # 收集共享库及其符号链接信息
    collect_opencv_lib(${CUSTOM_OPENCV_DIR} ${CMAKE_SHARED_LIBRARY_SUFFIX} "opencv_core" opencv_core_miss_lib opencv_core_regular opencv_core_symlink)
    collect_opencv_lib(${CUSTOM_OPENCV_DIR} ${CMAKE_SHARED_LIBRARY_SUFFIX} "opencv_imgcodecs" opencv_imgcodecs_miss_lib opencv_imgcodecs_regular opencv_imgcodecs_symlink)
    collect_opencv_lib(${CUSTOM_OPENCV_DIR} ${CMAKE_SHARED_LIBRARY_SUFFIX} "opencv_imgproc" opencv_imgproc_miss_lib opencv_imgproc_regular opencv_imgproc_symlink)
    
    if(opencv_core_miss_lib OR opencv_imgcodecs_miss_lib OR opencv_imgproc_miss_lib)
        set(OPENCV_MISS_LIBS TRUE)
    endif()
    
    # 有缺失就尝试opencv_world
    if(DEFINED OPENCV_MISS_LIBS AND OPENCV_MISS_LIBS)
        message(STATUS "Missing OpenCV independent shared libraries, try opencv_world")
        collect_opencv_lib(${CUSTOM_OPENCV_DIR} ${CMAKE_SHARED_LIBRARY_SUFFIX} "opencv_world" opencv_world_miss_lib opencv_world_regular opencv_world_symlink)
        if(opencv_world_miss_lib)
            message(FATAL_ERROR "Cannot find OpenCV independent shared libraries or opencv_world. Please make sure your custom OpenCV directory contains the missing libraries.")
        else()
            message(STATUS "Found opencv_world")
            list(APPEND LIBS_TO_INSTALL ${opencv_world_regular})
            list(JOIN opencv_world_symlink "/" opencv_world_symlink_str)
            list(APPEND SYMS_TO_INSTALL ${opencv_world_symlink_str})
        endif()
    
    # 找到了就添加进变量里
    else()
        # 这里因为cmake无法储存list(list(str))，因此我们把list给合成一个str再APPEND进SYMS_TO_INSTALL，
        # 之后要用时再拆回list。上面的opencv_world的处理方式也是一样的。输出的str长这样：
        # "${OpenCV库普通文件名}/${OpenCV库符号链接名1}/${OpenCV库符号链接名2}/..."
        
        list(APPEND LIBS_TO_INSTALL ${opencv_core_regular})
        list(JOIN opencv_core_symlink "/" opencv_core_symlink_str)
        list(APPEND SYMS_TO_INSTALL ${opencv_core_symlink_str})
        
        list(APPEND LIBS_TO_INSTALL ${opencv_imgcodecs_regular})
        list(JOIN opencv_imgcodecs_symlink "/" opencv_imgcodecs_symlink_str)
        list(APPEND SYMS_TO_INSTALL ${opencv_imgcodecs_symlink_str})
        
        list(APPEND LIBS_TO_INSTALL ${opencv_imgproc_regular})
        list(JOIN opencv_imgproc_symlink "/" opencv_imgproc_symlink_str)
        list(APPEND SYMS_TO_INSTALL ${opencv_imgproc_symlink_str})
    endif()
endif()

# 最后把所有要安装的共享库复制到TARGET的输出文件夹里
# 在之后的安装环节会直接从这个文件夹安装
foreach(ITEM ${LIBS_TO_INSTALL})
    add_custom_command(TARGET ${DEMO_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ITEM}
        "$<TARGET_FILE_DIR:${DEMO_NAME}>"
    )
endforeach()


# CMake TARGET安装路径设置
# 参考OpenCV：
# https://github.com/opencv/opencv/blob/a03b81316782ae30038b288fd3568993fa1e3538/CMakeLists.txt#L141-L152
# https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT.html
# 如果CMake安装路径为默认
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # 本地编译
    if(NOT CMAKE_TOOLCHAIN_FILE)
        if(WIN32)
            set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "安装路径" FORCE)
        else()
            set(CMAKE_INSTALL_PREFIX "/usr/" CACHE PATH "安装路径" FORCE)
        endif()
    
    # 其他跨平台编译
    else()
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "安装路径" FORCE)
    endif()
endif()

# 设置安装文件的运行环境路径
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# 安装TARGET
install(
    TARGETS ${DEMO_NAME}
    CONFIGURATIONS Debug Release
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
)
# 其他安装
if(WIN32)
    # 安装共享库
    install(
        DIRECTORY "$<TARGET_FILE_DIR:${DEMO_NAME}>/"
        DESTINATION "bin"
        FILES_MATCHING
        PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
        PATTERN "${DEMO_NAME}*" EXCLUDE
    )
else()
    # 安装共享库
    install(
        DIRECTORY "$<TARGET_FILE_DIR:${DEMO_NAME}>/"
        DESTINATION "lib"
        FILES_MATCHING
        PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}*"
        PATTERN "${DEMO_NAME}*" EXCLUDE
    )
    
    # 创建共享库的符号链接
    foreach(ITEM_STR ${SYMS_TO_INSTALL})
        string(REPLACE "/" ";" ITEM_LIST ${ITEM_STR})
        
        # 这里本来想用cmake 3.15的list(POP_FRONT)功能的，
        # 不过将cmake最低版本要求提到3.15会出现大量链接错误（详情请看文件顶部）
        # 于是就用一套旧的API来实现了
        # list(POP_FRONT ITEM_LIST ITEM_REGULAR_NAME)
        list(GET ITEM_LIST 0 ITEM_REGULAR_NAME)
        list(REMOVE_AT ITEM_LIST 0)
        
        foreach(ITEM_SYMLINK_NAME ${ITEM_LIST})
            # 这里我们在 cmake_install.cmake 安装脚本里加入下面这行脚本：
            # 进入到安装脚本环境里的 ${CMAKE_INSTALL_PREFIX}/lib 目录 
            # 然后再创建符号链接
            install(CODE "\
                execute_process(COMMAND ${CMAKE_COMMAND} -E chdir \$\{CMAKE_INSTALL_PREFIX\}/lib \
                ${CMAKE_COMMAND} -E create_symlink ${ITEM_REGULAR_NAME} ${ITEM_SYMLINK_NAME})"
            )
        endforeach()
    endforeach()
    
    if (INSTALL_WITH_TOOLS)
        # 安装启动脚本和安装脚本
        install(
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/linux_dist_tools/run.sh"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/linux_dist_tools/install_env.sh"
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
            DESTINATION "."
        )
        # 安装README.txt
        install(
            FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/linux_dist_tools/README.txt"
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            DESTINATION "."
        )
    endif()
endif()
