#! /bin/bash -e

# 获取当前脚本路径
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# 获取PaddleOCR-json路径
EXE_LOCATION="$SCRIPT_DIR/../build/bin/PaddleOCR-json"

# 所有PaddleOCR运行库都被复制到PaddleOCR-json的相同路径下了
LIBS="$(dirname $EXE_LOCATION)"

# 运行PaddleOCR-json
LD_LIBRARY_PATH="$LIBS" "$EXE_LOCATION" \
    "$@"
