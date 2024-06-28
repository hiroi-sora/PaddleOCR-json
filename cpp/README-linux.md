# PaddleOCR-json V1.3 Linux 构建指南

本文档帮助如何在Linux上编译 PaddleOCR-json V1.3 （对应PaddleOCR v2.6）。推荐给具有一定Linux命令行使用经验的读者。

本文参考了 PaddleOCR官方的[编译指南](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.6/deploy/cpp_infer/readme_ch.md) ，但建议以本文为准。

另外，本文将使用Debian/Ubuntu系列linux为例子进行讲解。其他linux发行版的用户请自行替换一些对应的命令（比如apt这类的）。

相关文档：
- [Windows 构建指南](./README.md)
- [Docker 部署](./README-docker.md)
- 其他平台 [移植指南](docs/移植指南.md)

## 1. 前期准备

### 1.0 兼容性检查：

**PaddleOCR-json 只支持具有AVX指令集的CPU。更多细节请查看[CPU要求](../README.md#离线ocr组件-系列项目)和[兼容性](../README.md#兼容性)。**

请先检查你的CPU兼容性：

```sh
lscpu | grep avx
```

**如果你的CPU支持AVX指令集，你的输出大概长这样（你可以在输出里找到 `avx` 的字符）：**

```
Flags:                              fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pdpe1gb rdtscp lm constant_tsc rep_good nopl xtopology tsc_reliable nonstop_tsc cpuid pni pclmulqdq vmx ssse3 fma cx16 sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowprefetch ssbd ibrs ibpb stibp ibrs_enhanced tpr_shadow vnmi ept vpid ept_ad fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid rdseed adx smap clflushopt clwb sha_ni xsaveopt xsavec xgetbv1 xsaves avx_vnni umip waitpkg gfni vaes vpclmulqdq rdpid movdiri movdir64b fsrm md_clear serialize flush_l1d arch_capabilities
```

**如果你看不到任何输出，这表明你的CPU不支持AVX指令集。**

> [!TIP]
> 如果你的CPU不支持AVX指令集，我们建议你尝试隔壁的[RapidOCR-json](https://github.com/hiroi-sora/RapidOCR-json)
>
> 当然，你也可以更换一个不需要AVX指令集的预测库来编译PaddleOCR-json（比如 `manylinux_cpu_noavx_openblas_gcc8.2` ）。不过大概率运行不了。

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

* [可选] 这一步之后可以根据预测库的版本来重命名一下 `paddle_inference` 文件夹。这里我们用的是 `manylinux_cpu_avx_mkl_gcc8.2` 的版本。

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
export PADDLE_LIB="$(pwd)/$(ls -d *paddle_inference*/ | head -n1)"
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

* 这里我们使用 `-S .` 命令来指定当前文件夹 `PaddleOCR-json/cpp` 为CMake项目根文件夹
* 使用 `-B build/` 命令来指定 `PaddleOCR-json/cpp/build` 文件夹为工程文件夹
* `-DPADDLE_LIB=$PADDLE_LIB` 命令会使用刚才设置的环境变量 `$PADDLE_LIB` 去指定预测库的位置
* 最后，`-DCMAKE_BUILD_TYPE=Release` 命令会将这个工程设置为 `Release` 工程。你也可以把它改成 `Debug`。

3. 使用 CMake 编译项目

```sh
cmake --build build/
```

* 这里我们使用 `--build build/` 命令来指定要编译的工程文件夹 `build`。

#### CMake构建参数

你可以使用 `-D参数名=值` 来添加新的CMake参数。

以下参数是一些编译参数：

| 参数名            | 描述                                                             |
| ----------------- | ---------------------------------------------------------------- |
| `WITH_MKL`        | 使用MKL或OpenBlas，默认使用MKL。                                 |
| `WITH_GPU`        | 使用GPU或CPU，默认使用CPU。                                      |
| `WITH_STATIC_LIB` | 编译成static library或shared library，默认编译成static library。 |
| `WITH_TENSORRT`   | 使用TensorRT，默认关闭。                                         |

> [!NOTE]
> * `WITH_STATIC_LIB`: Linux下这个参数设置成 `ON` 时无法编译，所以它是强行设置成 `OFF` 的。

以下是一些依赖库路径相关参数。除了 `PADDLE_LIB` 是必填的以外其他的视情况而定。

| 参数名         | 描述                         |
| -------------- | ---------------------------- |
| `PADDLE_LIB`   | paddle_inference的路径       |
| `OPENCV_DIR`   | 库的路径                     |
| `CUDA_LIB`     | 库的路径                     |
| `CUDNN_LIB`    | 库的路径                     |
| `TENSORRT_DIR` | 使用TensorRT编译并设置其路径  |

> [!NOTE]
> * `OPENCV_DIR`: Linux下，如果已经安装到系统之中就不用指定了。

以下是一些PaddleOCR-json功能相关参数。

| 参数名                   | 描述                               |
| ------------------------ | ---------------------------------- |
| `ENABLE_CLIPBOARD`       | 启用剪贴板功能。默认关闭。         |
| `ENABLE_REMOTE_EXIT`     | 启用远程关停引擎进程命令。默认开启。 |
| `ENABLE_JSON_IMAGE_PATH` | 启用json命令image_path。默认开启。 |

> [!NOTE]
> * `ENABLE_CLIPBOARD`: Linux下没有剪贴板功能，启用了也无法使用。
> * `ENABLE_REMOTE_EXIT`: 这个参数控制着 “[传入 `exit` 关停引擎进程](../docs/详细使用指南.md#4-关闭引擎进程)” 的功能。
> * `ENABLE_JSON_IMAGE_PATH`: 这个参数控制着 “使用`{"image_path":""}`指定路径” 的功能。

以下是一些CMake功能相关参数。

| 参数名                   | 描述                               |
| ------------------------ | --------------------------------- |
| `INSTALL_WITH_TOOLS`     | CMake安装时附带工具文件。默认开启。  |

#### 关于剪贴板读取

在Linux下，从剪贴板中读取数据的功能不存在。即使把 `ENABLE_CLIPBOARD` 设置成 `ON` 也无法使用。

#### 构建 or 编译失败？

如果报错中含有 `unable to access 'https://github.com/LDOUBLEV/AutoLog.git/': gnutls_handshake() failed: The TLS connection was non-properly terminated.` ，原因是网络问题，请挂全局科学上网。如果没有科学，那么可尝试将 `deploy/cpp_infer/external-cmake/auto-log.cmake` 中的github地址改为 `https://gitee.com/Double_V/AutoLog` 。

欢迎提出Issue。


## 3. 配置 & 运行可执行文件

1. 到这一步，你应该可以在 `build/bin/` 文件夹下找到一个叫 `PaddleOCR-json` 的可执行文件

```sh
ls ./build/bin/PaddleOCR-json
```

2. 直接运行的话会得到这样一个错误

```sh
./build/bin/PaddleOCR-json
```

```
./build/bin/PaddleOCR-json: error while loading shared libraries: libiomp5.so: cannot open shared object file: No such file or directory
```

> [!NOTE]
> 这是因为系统没法在环境变量 `LD_LIBRARY_PATH` 里列出的路径下找到上面这个共享库 `libiomp5.so`。

3. 这里我们直接更新环境变量 `LD_LIBRARY_PATH` 来解决。

```sh
# 所有的预测库共享库都已经被自动复制到 "build/bin" 文件夹下了，这里我们把它存到一个变量里。
LIBS="$(pwd)/build/bin/"
LD_LIBRARY_PATH=$LIBS ./build/bin/PaddleOCR-json
```

> [!TIP]
> 
> [什么是LD_LIBRARY_PATH？](https://developer.aliyun.com/article/1269445)
> 
> [使用LD_LIBRARY_PATH的风险？](https://m.ituring.com.cn/article/22101)

> [!NOTE]
> 如果你打算长期使用PaddleOCR-json的话，可以参考[安装章节](#5-安装)。

4. 到这一步，PaddleOCR-json 已经可以运行了。不过它会提示你缺少配置文件。我们所需的所有文件都在之前准备的模型库 `module` 文件夹里面。

```sh
LD_LIBRARY_PATH=$LIBS ./build/bin/PaddleOCR-json \
    -models_path="$MODELS" \
    -config_path="$MODELS/config_chinese.txt" \
    -image_path="/path/to/image.jpg" # 图片的路径
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
./tools/linux_run.sh [配置参数]
```

* [常用配置参数](../README.md#常用配置参数说明)

## 5. 安装

你可以使用CMake来安装PaddleOCR-json到系统里。直接以 `sudo` 权限运行下面这条命令。

```sh
sudo cmake --install build
```

CMake会将 `build` 文件夹下的可执行文件和运行库给安装到系统文件夹 `/usr/` 下，这样你就可以直接用 `PaddleOCR-json` 来调用这个软件了。

如果你希望安装到指定位置，你可以为上面这条命令加上参数 `--prefix /安装路径/` 来指定一个安装路径。比如 `--prefix build/install` 会将所有的文件都安装到 `build/install` 文件夹下。

> [!TIP]
> 在Linux下安装时，CMake会额外安装一些工具脚本和文档以方便用户直接使用（[就是 `linux_dist_tools/` 文件夹下的东西](./tools/linux_dist_tools/)）。这个功能可以帮助开发者更方便的打包软件。但是，如果你希望将PaddleOCR-json安装到系统文件夹里，你则不需要这些工具文件。你可以通过关闭CMake参数 `INSTALL_WITH_TOOLS` 来禁用这些工具文件的安装。

> [!TIP]
> CMake在安装PaddleOCR-json时，会将所有在 `build/bin` 文件夹下的共享依赖库给复制到安装目录的 `lib` 文件夹下。但是，Linux的很多共享库是被拆分在系统文件夹里的（比如 `/usr/lib/` ）。CMake无法自动找到这些共享依赖库。如果你需要将PaddleOCR-json打包成一个无依赖的软件，你需要手动将所需的共享依赖库从系统文件夹里找出并复制到 `build/bin` 文件夹下。这样一来CMake就可以在安装时将完整的共享依赖库一起打包了。

## 6. 其他问题

### [关于内存泄漏 / 长期高内存占用](./README.md#关于内存泄漏--长期高内存占用)
