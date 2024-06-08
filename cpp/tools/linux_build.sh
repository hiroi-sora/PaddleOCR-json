#! /bin/bash -e

# 获取当前脚本路径并去到 "cpp" 文件夹下
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR/..

# 一些CMake的变量
MODE=Release
PADDLE_LIB="$(pwd)/$(ls -d .source/*paddle_inference*/ | head -n1)"

echo "MODE: $MODE"
echo "PADDLE_LIB: $PADDLE_LIB"

# 构建 & 编译项目
mkdir -p build

# 如果build文件夹不是一个CMake工程，则构建工程文件夹（第一次运行）
if [ ! -d "build/CMakeFiles/" ]; then
    cmake -S . -B build/ -DPADDLE_LIB="$PADDLE_LIB" -DCMAKE_BUILD_TYPE="$MODE"
fi

# 编译
cmake --build build/ --config="$MODE"
