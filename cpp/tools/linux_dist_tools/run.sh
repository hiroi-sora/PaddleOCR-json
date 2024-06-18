#! /bin/bash -e

# 获取当前脚本路径
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# 获取PaddleOCR-json路径
EXE_LOCATION="$SCRIPT_DIR/bin/PaddleOCR-json"
LIB_LOCATION="$SCRIPT_DIR/lib/"

# 运行PaddleOCR-json
LD_LIBRARY_PATH="$LIB_LOCATION" "$EXE_LOCATION" -models_path="$SCRIPT_DIR/models" "$@"
