# PaddleOCR-json V1.3 Linux 构建指南

本文档帮助如何在Linux上编译 PaddleOCR-json V1.3 （对应PPOCR v2.6）。推荐给具有一定Linux命令行使用经验的读者。

本文参考了 PPOCR官方的[编译指南](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.6/deploy/cpp_infer/readme_ch.md) ，但建议以本文为准。

另外，本文将使用Debian/Ubuntu系列linux为例子进行讲解。其他linux发行版的用户请自行替换一些对应的命令（比如apt这类的）。

[Windows 构建指南](./README.md)

移植多平台，请参考 [移植指南](docs/移植指南.md) 。


## 1. 前期准备

### 1.1 需要安装的工具：

- wget（下载预测库用）
- tar、zip、unzip（解压软件）
- git
- gcc 和 g++
- cmake 和 make
- libopencv-dev（OpenCV开发工具，PaddleOCR官方推荐至少是3.4.7版本）

安装以上工具

```sh
sudo apt install wget tar zip unzip git gcc g++ cmake make libopencv-dev
```

> [!TIP]
> 如果你需要自行编译OpenCV，可以参考[OpenCV官方文档](https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html)

### 1.2 需要下载的资源：

- [paddle_inference](https://paddleinference.paddlepaddle.org.cn/user_guides/download_lib.html#linux) (Linux, 2.3.2, C++预测库, gcc编译器版本, manylinux_cpu_avx_mkl_gcc8.2)
- [模型库](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/models%2Fv1.3) (models.zip)

### 1.3 放置资源

1. clone 本仓库。

```sh
git clone https://github.com/hiroi-sora/PaddleOCR-json.git
cd PaddleOCR-json
```

2. 在 `PaddleOCR-json/cpp` 下新建一个文件夹 `.source` 来存放外部资源（前面加点是为了按文件名排列更顺眼）。如果 `.source` 文件夹已经存在则不需要创建。
```sh
mkdir -p cpp/.source
cd cpp/.source
```

3. 下载paddle_inference并解压到 `.source` 文件夹下。

```sh
wget https://paddle-inference-lib.bj.bcebos.com/2.3.2/cxx_c/Linux/CPU/gcc8.2_avx_mkl/paddle_inference.tgz
tar -xf paddle_inference.tgz
```

* 这一步之后可以根据预测库的版本来重命名一下 `paddle_inference` 文件夹。这里我们用的是 `manylinux_cpu_avx_mkl_gcc8.2` 的版本。

```sh
mv paddle_inference paddle_inference_manylinux_cpu_avx_mkl_gcc8.2
```

4. 下载模型库并解压到 `.source` 文件夹下。

```sh
wget https://github.com/hiroi-sora/PaddleOCR-json/releases/download/models%2Fv1.3/models.zip
unzip -x models.zip
```

完成后应该是这样：
```
PaddleOCR-json
└─ cpp
    ├─ .source
    │    ├─ models
    │    └─ paddle_inference_manylinux_cpu_avx_mkl_gcc8.2
    ├─ CMakeLists.txt
    ├─ README.md
    ├─ docs
    ├─ external-cmake
    ├─ include
    └─ src
```

5. 最后一步，为了方便之后的使用，设置两个环境变量。

```sh
export PADDLE_LIB="$(pwd)/paddle_inference_manylinux_cpu_avx_mkl_gcc8.2"
export MODELS="$(pwd)/models"
```

可以用echo来检查一下

```sh
echo $PADDLE_LIB
echo $MODELS
```


## 2. 构建 & 编译项目

1. 在 `PaddleOCR-json/cpp` 下，新建一个文件夹 `build`

```sh
mkdir build
```

2. 使用 CMake 构建项目

```sh
cmake -S . -B build/ \
    -DPADDLE_LIB=$PADDLE_LIB \
    -DCMAKE_BUILD_TYPE=Release
```

* 这里我们使用 `-S .` 命令来指定当前文件夹 `PaddleOCR-json/cpp` 为源码文件夹
* 使用 `-B build/` 命令来指定 `PaddleOCR-json/cpp/build` 文件夹为工程文件夹
* `-DPADDLE_LIB=$PADDLE_LIB` 命令会使用刚才设置的环境变量 `$PADDLE_LIB` 去指定预测库的位置
* 最后，`-DCMAKE_BUILD_TYPE=Release` 命令会将这个工程设置为 `Release` 工程。你也可以把它改成 `Debug`。

> [!NOTE]
> 在构建工程文件夹时，CMake可能会报告下面这一套错误：
> 
> `cp: cannot create regular file '/usr/lib/libmklml_intel.so': Permission denied`
>
> 这是因为PaddleOCR官方的CMakeLists会尝试安装预测库自带的 `mklml` 库到系统的 `/usr/lib` 下面，然后因为CMake没有权限而失败。
>
> 如果你不了解Linux权限相关知识，或者不明白系统的 `/usr/lib` 有什么作用，请直接无视这条错误。不用去处理它。
> 
> 如果你明白以上两点并且愿意在系统里手动安装第三方库的话，你可以在上面的CMake命令前加上 `sudo` 来提权（不建议这么做）。

3. 使用 CMake 编译项目

```sh
cmake --build build/
```

* 这里我们使用 `--build build/` 命令来指定要编译的工程文件夹 `build`。

#### 构建 or 编译失败？

如果报错中含有 `unable to access 'https://github.com/LDOUBLEV/AutoLog.git/': gnutls_handshake() failed: The TLS connection was non-properly terminated.` ，原因是网络问题，请挂全局科学上网。如果没有科学，那么可尝试将 `deploy/cpp_infer/external-cmake/auto-log.cmake` 中的github地址改为 `https://gitee.com/Double_V/AutoLog` 。

欢迎提出Issue


## 3. 配置 & 使用编译出来的可执行文件

1. 到这一步，你应该可以在 `build` 文件夹下找到一个叫 `ppocr` 的可执行文件

```sh
ls ./build/ppocr
```

2. 直接运行的话会得到这样一个错误

```sh
./build/ppocr
```

```
./build/ppocr: error while loading shared libraries: libiomp5.so: cannot open shared object file: No such file or directory
```

> [!NOTE]
> 这是因为系统没法在环境变量 `PATH` 里列出的路径下找到上面这个共享库 `libiomp5.so`。这就是为什么之前PaddleOCR想在系统路径下安装自带的第三方库了

3. 一般我们可以更新环境变量 `PATH` 来解决这个问题，不过更新 `PATH` 有些时候不一定会起效。这里我们直接更新另一个环境变量 `LD_LIBRARY_PATH` 来解决。

```sh
# 从之前的预测库文件夹下找出所有名为 "lib" 的文件夹，然后再把他们用 ":" 字符给串接起来。（就是 String.join()）最后在存到一个变量里面。
LIBS="$(find $PADDLE_LIB -name 'lib' -type d | paste -sd ':' -)"
LD_LIBRARY_PATH=$LIBS ./build/ppocr
```

> [!TIP]
> 
> [什么是LD_LIBRARY_PATH？](https://developer.aliyun.com/article/1269445)
> 
> [使用LD_LIBRARY_PATH的风险？](https://m.ituring.com.cn/article/22101)

> [!NOTE]
> 如果你打算长期使用PaddleOCR-json的话，建议还是老实的在系统环境下安装所有的依赖库吧。
>
> 你可以按照[构建 & 编译项目第二条](#2-构建--编译项目)下面提到的那样直接给CMake提权让它来安装。
>
> 或者你也可以使用系统的包管理器来安装

4. 到这一步，PaddleOCR-json 已经可以运行了。不过它会提示你缺少配置文件。我们所需的所有文件都在之前准备的模型库 `module` 文件夹里面。

```sh
# PaddleOCR-json 必须运行在 "module" 文件夹的相同目录下
cd $MODELS/..
LD_LIBRARY_PATH=$LIBS ../build/ppocr \
    -config_path=./models/config_chinese.txt \
    -image_path=/path/to/image.jpg # 图片的路径
```

> [!TIP]
> 更多配置参数请参考[简单试用](../README.md#简单试用)和[常用配置参数说明](../README.md#常用配置参数说明)


## 4. 一键编译 + 运行

我们准备了两个简单的脚本方便一键编译、运行PaddleOCR-json。

> [!WARNING]
> 请注意，下面这些脚本并没有安装 PaddleOCR-json 到你的系统里。不适合想要长期使用的用户。只是方便开发者的重复编译、测试

### 4.1 一键编译

在完成[第一大章前期准备](#1-前期准备)之后，你可以使用下面这个脚本来直接构建 + 编译工程。

```sh
./tools/linux_build.sh
```

### 4.2 一键运行

在编译完成后（[完成第二大章之后](#2-构建--编译项目)），你可以使用下面这个脚本来直接运行PaddleOCR-json

```sh
./tools/linux_run.sh /图片路径/img.jpg
```
