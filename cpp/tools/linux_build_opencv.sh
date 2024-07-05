#! /bin/bash -e

# 当前脚本必须在 cpp/.source 工作目录下执行
source_dir=$(pwd)  # 当前工作目录

# 获取 OpenCV 源码目录（可相对工作目录）
opencv_dir=$1

opencv_dir=$(realpath "$opencv_dir")
cd "$opencv_dir"

build_dir="${opencv_dir}/build"  # 构建目录
release_dir="${source_dir}/opencv-release"  # 生成目录

rm -rf ${build_dir}
rm -rf ${release_dir}
mkdir ${build_dir}
cd ${build_dir}

cmake ../ \
    -DCMAKE_INSTALL_PREFIX=${release_dir} \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DWITH_IPP=OFF \
    -DBUILD_IPP_IW=OFF \
    -DWITH_LAPACK=OFF \
    -DWITH_EIGEN=OFF \
    -DCMAKE_INSTALL_LIBDIR=lib64 \
    -DWITH_ZLIB=ON \
    -DBUILD_ZLIB=ON \
    -DWITH_JPEG=ON \
    -DBUILD_JPEG=ON \
    -DWITH_PNG=ON \
    -DBUILD_PNG=ON \
    -DWITH_TIFF=ON \
    -DBUILD_TIFF=ON

    # -DBUILD_LIST=core,imgcodecs,imgproc \ # 加了后似乎有问题？

make -j
make install
