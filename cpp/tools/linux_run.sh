#! /bin/bash -e

# 如果没有配置参数
if [ $# -eq 0 ]; then
    echo "用法：./tools/linux_run.sh [配置参数]"
    exit 0
fi

# 获取当前脚本路径
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# 获取PaddleOCR-json路径
EXE_LOCATION="$SCRIPT_DIR/../build/bin/PaddleOCR-json"

# 所有PaddleOCR运行库都被复制到PaddleOCR-json的相同路径下了
LIBS="$(dirname $EXE_LOCATION)"

# echo "LIBS: $LIBS"

# 运行PaddleOCR-json
LD_LIBRARY_PATH="$LIBS" "$EXE_LOCATION" \
    "$@"
