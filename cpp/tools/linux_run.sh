#! /bin/bash -e

# 没有配置参数
if [ $# -eq 0 ]; then
    echo "用法：./tools/linux_run.sh [配置参数]"
    echo "请注意：所有的相对路径都将以 .source 文件夹为基准"
    exit 0
fi

# 获取PaddleOCR-json路径
EXE_LOCATION="$(pwd)/build/bin/PaddleOCR-json"

# 获取当前脚本路径并去到 "cpp/.source" 文件夹下
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd "$SCRIPT_DIR/../.source"

# 所有PaddleOCR运行库都被复制到PaddleOCR-json的相同路径下了
LIBS="$(dirname $EXE_LOCATION)"

echo "LIBS: $LIBS"

# 运行PaddleOCR-json
LD_LIBRARY_PATH="$LIBS" "$EXE_LOCATION" \
    "$@"
