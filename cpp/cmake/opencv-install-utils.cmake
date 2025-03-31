# OpenCV 安装工具函数

# collect_opencv_lib()
# @参数
#   in_opencv_lib_dir               [输入][str]  用户输入的 OPENCV_DIR
#   in_cmake_shared_lib_suffix      [输入][str]  CMAKE_SHARED_LIBRARY_SUFFIX
#   in_opencv_lib_name              [输入][str]  当前链接的OpenCV库名，如 opencv_core、opencv_imgcodecs
#   out_opencv_miss_lib             [输出][bool] 是否找到了输入的OpenCV库文件
#   out_opencv_lib_regular_file     [输出][str]  找到的OpenCV库普通文件绝对路径
#   out_opencv_lib_symlink          [输出][list] 找到的OpenCV库普通文件与库符号链接名，用于后续创建符号链接
macro(collect_opencv_lib
        in_opencv_lib_dir
        in_cmake_shared_lib_suffix
        in_opencv_lib_name
        out_opencv_miss_lib
        out_opencv_lib_regular_file
        out_opencv_lib_symlink
    )
    # 寻找库文件
    file(GLOB_RECURSE __opencv_libs_found "${in_opencv_lib_dir}/**/*${in_opencv_lib_name}*${in_cmake_shared_lib_suffix}*")
    
    # 检查是否找到
    if("${__opencv_libs_found}" STREQUAL "")
        message(STATUS "Missing independent shared library: ${in_opencv_lib_name}")
        set(${out_opencv_miss_lib} TRUE)
    endif()
    
    # 拆分普通文件与符号链接
    foreach(__item ${__opencv_libs_found})
        if(IS_SYMLINK ${__item})
            get_filename_component(__item_name ${__item} NAME)
            list(APPEND __symlink_libs ${__item_name})
        else()
            set(__regular_libs ${__item})
        endif()
    endforeach()
    
    # 输出
    set(${out_opencv_lib_regular_file} ${__regular_libs})
    if(DEFINED __symlink_libs AND NOT "${__symlink_libs}" STREQUAL "")
        get_filename_component(__regular_libs_name "${__regular_libs}" NAME)
        set(${out_opencv_lib_symlink} ${__regular_libs_name} ${__symlink_libs})
    endif()
    
    # 清理缓存的变量
    unset(__opencv_libs_found)
    unset(__item)
    unset(__item_name)
    unset(__symlink_libs)
    unset(__regular_libs)
    unset(__regular_libs_name)
endmacro(collect_opencv_lib)
