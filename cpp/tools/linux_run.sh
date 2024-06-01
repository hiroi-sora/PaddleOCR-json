#! /bin/bash -e

# 检查第一个argument
if [ -z "$1" ] ; then
    echo "用法：./tools/linux_run.sh /图片路径/img.jpg"
    exit
fi

# 将第一个argument转成绝对路径
if [[ "$1" = /* ]]; then # 绝对路径
    IMG_PATH=$1
else # 相对路径
    IMG_PATH="$(pwd)/$1"
fi

echo "输入图片路径：$IMG_PATH"


# 获取当前脚本路径并去到 "cpp/.source" 文件夹下
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR/../.source

# 从预测库里找出所有动态库文件夹
PADDLE_LIB="paddle_inference"
LIBS="$(find $PADDLE_LIB -name 'lib' -type d | paste -sd ':' -)"

# 运行PaddleOCR-json
LD_LIBRARY_PATH=$LIBS ../build/ppocr \
    -config_path=./models/config_chinese.txt \
    -image_path=$IMG_PATH
