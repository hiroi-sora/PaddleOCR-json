#! /bin/bash -e

# 获取当前脚本路径
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# 获取PaddleOCR-json路径
EXE_LOCATION="$SCRIPT_DIR/bin/"
LIB_LOCATION="$SCRIPT_DIR/lib/"

# 将PaddleOCR-json及其依赖的路径写入 ~/.bashrc
eval echo 'export PATH=$PATH:$EXE_LOCATION' >> ~/.bashrc
eval echo 'export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$LIB_LOCATION' >> ~/.bashrc

echo ""
echo "You need to restart current shell session, or run following command in order for the install to take effect:"
echo "你需要重启当前shell session，或者运行下面这条命令来完成安装步骤:"
echo ""
echo "source ~/.bashrc"
echo ""
