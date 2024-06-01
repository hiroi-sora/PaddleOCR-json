#! /bin/bash -e

# 获取当前脚本路径并去到 "cpp" 文件夹下
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR/..

# 一些CMake的变量
MODE=Release
PADDLE_LIB="./.source/paddle_inference"

# 构建 & 编译项目
mkdir -p build
cmake -S . -B build/ -DPADDLE_LIB=$PADDLE_LIB -DCMAKE_BUILD_TYPE=$MODE
cmake --build build/ --config=$MODE
