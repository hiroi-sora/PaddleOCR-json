#! /bin/bash -e

# 当前脚本必须在 cpp/.source 工作目录下执行
source_dir=$(pwd)  # 当前工作目录

# 获取 OpenCV 源码目录（可相对工作目录）
if [ $# -lt 1 ]; then
    echo "Usage: build_opencv.sh /path/to/opencv"
    exit 1
fi
opencv_dir=$1
opencv_dir=$(realpath "$opencv_dir")
cd "$opencv_dir"

build_dir="${opencv_dir}/build"  # 构建目录
release_dir="${source_dir}/opencv-release"  # 生成目录

if [ -d ${build_dir} ]; then
    rm -rf ${build_dir}
    echo "Delete: ${build_dir}"
fi
if [ -d ${release_dir} ]; then
    rm -rf ${release_dir}
    echo "Delete: ${release_dir}"
fi
mkdir ${build_dir}
echo "Create: ${build_dir}"
cd ${build_dir}

cmake ../ \
    -DCMAKE_INSTALL_PREFIX=${release_dir} \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_LIST=core,imgcodecs,imgproc \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_opencv_world=OFF \
    -DOPENCV_FORCE_3RDPARTY_BUILD=ON \
    -DWITH_ZLIB=ON \
    -DWITH_TIFF=ON \
    -DWITH_OPENJPEG=ON \
    -DWITH_JASPER=ON \
    -DWITH_JPEG=ON \
    -DWITH_PNG=ON \
    -DWITH_OPENEXR=ON \
    -DWITH_WEBP=ON \
    -DWITH_IPP=ON \
    -DWITH_LAPACK=ON \
    -DWITH_EIGEN=ON \
    -DBUILD_PERF_TESTS=OFF \
    -DBUILD_TESTS=OFF \
    -DBUILD_DOCSL=OFF \
    -DBUILD_JAVA=OFF \
    -DBUILD_opencv_python2=OFF \
    -DBUILD_opencv_python3=OFF


make -j
make install

