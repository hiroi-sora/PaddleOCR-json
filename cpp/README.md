# PaddleOCR-json V1.4 Windows 构建指南

> 注：此开发版基于 Paddle Inference 3.0.0 推理后端，在不带 AVX512 指令集的普通家用CPU上存在性能显著下降的问题。普通用户建议切换到本项目稳定版分支。

本文档帮助如何在Windows上编译 PaddleOCR-json V1.4 （对应PPOCR v2.8）。面向小白，用的最简单的步骤。大佬可酌情调整。

本文参考了 PPOCR官方的[编译指南](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.8/deploy/cpp_infer/docs/windows_vs2019_build.md) ，但建议以本文为准。

相关文档：
- [Linux 构建指南](./README-linux.md)
- [Docker 部署](./README-docker.md#使用-docker-部署)
- 其他平台 [移植指南](docs/移植指南.md)


## 1. 前期准备

资源链接后面的(括弧里是版本)，请看清楚。

### 1.1 需要安装的工具：

- [Visual Studio 2022](https://learn.microsoft.com/zh-cn/visualstudio/releases/2022/release-notes) (2022或2019均可，Community)
- [Cmake](https://cmake.org/download/) (Windows x64 Installer)
- [Git](https://git-scm.com/download/win) (64-bit Git for Windows Setup)

### 1.2 需要下载的资源：

- [paddle_inference.zip](https://paddle-inference-lib.bj.bcebos.com/3.0.0-beta1/cxx_c/Windows/CPU/x86-64_avx-mkl-vs2019/paddle_inference.zip) (Windows C++预测库, 3.0.0, cpu_avx_mkl)
- [Opencv](https://github.com/opencv/opencv/releases) (windows.exe)
- [模型库](https://github.com/hiroi-sora/PaddleOCR-json/releases/download/v1.4.1-dev/models_v1.4.1.zip) (models.zip)

### 1.3 放置资源

1. clone 本仓库。在 `PaddleOCR-json/cpp` 下新建一个文件夹 `.source` 来存放外部资源。（前面加点是为了按文件名排列更顺眼）
2. 将下载好的 `models_v1.4.1.zip` 、 `paddle_inference` 和 `Opencv` 解压进`.source`。
   - `paddle_inference` 应该解压后放入一个单独文件夹内，并且根据版本给文件夹改个后缀，比如是cpu_avx_mkl版，就叫 `paddle_inference_cpu_avx_mkl` ，以便区分。
   - Opencv看起来是个exe，实际上是个自解压包，运行并选择目录解压。

完成后应该是这样：
```
PaddleOCR-json
└─ cpp
    ├─ .source
    │    ├─ opencv
    │    ├─ models
    │    └─ paddle_inference_cpu_avx_mkl
    ├─ CMakeLists.txt
    ├─ README.md
    ├─ docs
    ├─ external-cmake
    ├─ include
    └─ src
```

## 2. 构建项目

1. cmake安装完后系统里会有一个cmake-gui程序，打开cmake-gui。在第一个输入框处填写源代码路径，第二个输入框处填写编译输出路径，见下面的模板。  
然后，点击左下角第一个按钮Configure，第一次点它会弹出提示框进行Visual Studio配置，选择你的Visual Studio版本（2019、2022均可），目标平台选择x64。然后点击finish按钮即开始自动执行配置。

Where is the source code: `……/PaddleOCR-json/cpp`

Where to build the binaries: `……/PaddleOCR-json/cpp/build`

![](docs/imgs/b1.png)

执行完会报错，很正常，点OK。
![](docs/imgs/b2.png)

2. 填写两项配置：

OPENCV_DIR:  
`……/PaddleOCR-json/cpp/.source/opencv/build/x64/vc16/lib`

PADDLE_LIB:  
`……/PaddleOCR-json/cpp/.source/paddle_inference_cpu_avx_mkl`

下面 `WITH_GPU` 确保**不要**勾选。

其他项就不要动了！

![](docs/imgs/b3.png)

点击左下角 **第一个按钮Configure** 应用配置，等待几秒，看到输出 `Configuring done` 即可。

点击左下角 **第二个按钮Generate** 即可生成Visual Studio 项目的sln文件。看到输出 `Generating done` 即可。那么，你会看到 `PaddleOCR-json/cpp/build` 下生成了 `PaddleOCR-json.sln` 及一堆文件。

#### CMake构建参数说明

像刚才我们勾选/填写的那些CMake选项（`WITH_GPU`、`OPENCV_DIR`、`PADDLE_LIB`这些），它们是CMake的参数。你也可以自行参考并修改这些参数。

以下参数是一些编译参数：

| 参数名            | 描述                                                                                                                                     |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| `WITH_MKL`        | 使用MKL或OpenBlas，默认使用MKL。                                                                                                         |
| `WITH_GPU`        | 使用GPU或CPU，默认使用CPU。                                                                                                              |
| `WITH_STATIC_LIB` | 编译成static library或shared library，默认编译成static library。（Linux下这个参数设置成 `ON` 时无法编译，所以它是强行设置成 `OFF` 的。） |
| `WITH_TENSORRT`   | 使用TensorRT，默认关闭。                                                                                                                 |

以下是一些依赖库路径相关参数。除了 `PADDLE_LIB` 是必填的以外其他的视情况而定。

| 参数名         | 描述                         |
| -------------- | ---------------------------- |
| `PADDLE_LIB`   | paddle_inference的路径       |
| `OPENCV_DIR`   | 库的路径                     |
| `CUDA_LIB`     | 库的路径                     |
| `CUDNN_LIB`    | 库的路径                     |
| `TENSORRT_DIR` | 使用TensorRT编译并设置其路径 |

> [!NOTE]
> * 您也可以通过设置环境变量 `OpenCV_DIR` 来设置OpenCV库的路径，注意变量名大小写敏感。

以下是一些PaddleOCR-json功能相关参数。

| 参数名                   | 描述                                 |
| ------------------------ | ------------------------------------ |
| `ENABLE_CLIPBOARD`       | 启用剪贴板功能。默认关闭。           |
| `ENABLE_REMOTE_EXIT`     | 启用远程关停引擎进程命令。默认开启。 |
| `ENABLE_JSON_IMAGE_PATH` | 启用json命令image_path。默认开启。   |

> [!NOTE]
> * `ENABLE_REMOTE_EXIT`: 这个参数控制着 “[传入 `exit` 关停引擎进程](../docs/详细使用指南.md#4-关闭引擎进程)” 的功能。
> * `ENABLE_JSON_IMAGE_PATH`: 这个参数控制着 “使用`{"image_path":""}`指定路径” 的功能。

#### 关于剪贴板读取

在Windows下，从剪贴板中读取数据的功能存在，不过已经弃用。所有剪贴板相关的代码是默认不编译的。如果你需要PaddleOCR-json去读取剪贴板，请自行修改CMake参数 `ENABLE_CLIPBOARD=ON` 并重新编译。

#### 构建失败？

如果报错中含有 `Could NOT find Git (missing: GIT_EXECUTABLE)` ，原因是电脑上未安装Git，请先安装（尽量装在默认目录下）。

如果报错中含有 `unable to access 'https://github.com/LDOUBLEV/AutoLog.git/': gnutls_handshake() failed: The TLS connection was non-properly terminated.` ，原因是网络问题，请挂全局科学上网。如果没有科学，那么可尝试将 `deploy/cpp_infer/external-cmake/auto-log.cmake` 中的github地址改为 `https://gitee.com/Double_V/AutoLog` 。

其他原因，请确认您操作的步骤与本文一致，尤其是点击按钮的先后顺序。

## 3. 配置项目

1. 回到工程目录下的build文件夹，打开 `PaddleOCR-json.sln` 。**将Debug改为Release**。

![](docs/imgs/b5.png)

2. 调整项目设置。

- 解决方案管理器 → **ALL_BUILD** → 右键 → 属性，进行修改：
  - 常规 → 输出目录：`$(ProjectDir)\bin\Release`
  - 调试 → 命令：`$(ProjectDir)\bin\Release\PaddleOCR-json.exe`
  - 调试 → 工作目录：`$(ProjectDir)\bin\Release`
- 解决方案管理器 → **PaddleOCR-json** → 右键 → 属性，进行修改：
  - 常规 → 输出目录：`$(ProjectDir)\bin\Release`

![](docs/imgs/b6.png)

3. 按F5编译。如果输出类似 `生成：成功4个，失败0个……` 的语句，然后弹出一个控制台窗口，并报错 `找不到 opencv_world***.dll` ，那么**编译正常**。你能在 `build/bin/Release` 下找到生成的 `PaddleOCR-json.exe` 。

![](docs/imgs/b7.png)

> [!TIP]
> 如果编译时，报了大量的语法错误，如：
> ```
> C2447 "{"缺少函数标题
> C2059 语法错误："if"
> C2143 语法错误：缺少";"
> …………等等
> ```
> 那么可能是源代码文件的换行符编码问题。  
> 解决方法一：通过`git clone`下载本仓库代码，而不要直接在Github下载zip文件包。  
> 解决方法二：批量将所有`.h`和`.cpp`文件的 [换行符转换为CRLF](https://www.bing.com/search?q=%E6%89%B9%E9%87%8F%E8%BD%AC%E6%8D%A2+LF+%E5%92%8C+CRLF) 。

4. 拷贝必要的运行库。将以下文件拷贝到 `build/bin/Release` 文件夹下。

- `opencv_world***.dll`: `PaddleOCR-json/cpp/.source/opencv/build/x64/vc16/bin/opencv_world***.dll`

> 当然，你也可以直接将 `opencv` 的运行库放到Windows的 `PATH` 环境变量中。参考[这篇文档](https://cloud.baidu.com/article/3297806)，把路径 `opencv/build/x64/vc16/bin/` 加入 `PATH`。这样就不需要拷贝 `opencv` 运行库了。

5. 拷贝模型库。将 `.source` 中的 `models` 整个拷贝到 `build/bin/Release` 文件夹下。

6. 在`build/bin/Release`下，Shift+右键，在此处打开终端（或PowerShell），输入 `./PaddleOCR-json.exe` 。如果输出下列文字，就正常。

```
OCR anonymous pipe mode.
OCR init time: 0.62887s
OCR init completed.
```

如果你需要移植其他平台，可以参考文档 [移植指南](docs/移植指南.md)

## 4. 安装

你可以使用CMake来安装PaddleOCR-json。在`cpp`下，Shift+右键，在此处打开终端（或PowerShell），运行下面这条命令。

```sh
cmake --install build
```

CMake会将 `build` 文件夹下的可执行文件和运行库给安装到 `build/install/bin` 文件夹下。CMake无法在Windows下把软件安装到系统文件夹中，不过你可以将文件夹 `cpp/build/install/bin` 添加到Windows的 `PATH` 环境变量中。参考[这篇文档](https://cloud.baidu.com/article/3297806)。

如果你希望安装到指定位置，你可以为上面这条命令加上参数 `--prefix /安装路径/` 来指定一个安装路径。比如 `--prefix build/install` 会将所有的文件都安装到 `build/install` 文件夹下。

## 5. 切换语言/模型库/预设

可通过启动时的命令行参数，指定使用模型库或者预设。

| 参数               | 说明                                                                         |
| ------------------ | ---------------------------------------------------------------------------- |
| det_model_dir      | 文本检测模型库路径。所有语言都能使用 `models/ch_PP-OCRv4_det_infer`          |
| cls_model_dir      | 方向分类模型库路径。所有语言都能使用 `models/ch_ppocr_mobile_v2.0_cls_infer` |
| rec_model_dir      | 文本识别模型库路径。不同语言应该使用不同的识别库。                           |
| rec_char_dict_path | 文本识别模型库对应的字典文件路径。                                           |
| rec_img_h          | 文本识别模型特殊参数。V2模型需手动设为32，V3/V4模型无需设置。                |

`det`, `cls`, `rec` 支持使用PP-OCR系列官方模型，或自己训练的符合PP规范的模型。支持 V2~V4 模型。

路径建议填相对路径，根为 PaddleOCR-json 目录。假设模型都存放在 `models` 目录中，那么可以设定这样的参数：

（Linux下， `PaddleOCR-json.exe` 换成 `run.sh`）

```sh
PaddleOCR-json.exe \
    --det_model_dir=models/ch_PP-OCRv4_det_infer \
    --cls_model_dir=models/ch_ppocr_mobile_v2.0_cls_infer \
    --rec_model_dir=models/ch_PP-OCRv4_rec_infer \
    --rec_char_dict_path=models/dict_chinese.txt
```

也可以将上述参数写进一个txt里，格式如：`config_chinese.txt`

```sh
# 这是单行注释

# det 检测模型库
det_model_dir models/ch_PP-OCRv4_det_infer
# cls 方向分类器库
cls_model_dir models/ch_ppocr_mobile_v2.0_cls_infer
# rec 识别模型库
rec_model_dir models/ch_PP-OCRv4_rec_infer
# 字典路径
rec_char_dict_path models/dict_chinese.txt
```

然后，将该txt传入 `config_path` 参数中：

```sh
PaddleOCR-json.exe --config_path=models/config_chinese.txt
```

我们提供的 `model.zip` 中已经包含了多种语言的预设文件：

| 语言     | 预设文件                        | 模型版本 |
| -------- | ------------------------------- | -------- |
| 简体中文 | `models/config_chinese.txt`     | V4       |
| English  | `models/config_en.txt`          | V4       |
| 繁體中文 | `models/config_chinese_cht.txt` | V2       |
| 日本語   | `models/config_japan.txt`       | V4       |
| 한국어   | `models/config_korean.txt`      | V4       |

（注：繁體中文使用V2版本模型，因为V2竖排识别的准确度比后续版本更好。）

更多 PP-OCR 系列官方模型下载： https://github.com/PaddlePaddle/PaddleOCR/blob/main/doc/doc_ch/models_list.md

## 6. 其他问题

### 关于内存占用

本项目后端推理库为 `Paddle Inference 3.0.0 beta.1 cpu_mkl` 。比起旧版本，`3.0.0` 大幅优化了内存占用率。进行大批量、连续OCR（约1000张图片）后，内存占用稳定在 1.5G 左右。

建议至少在 4G 内存的机器上使用本项目，预留 2G 空闲内存给本项目。

如果有必要在 2G 内存的机器上使用本项目，建议添加启动参数 `--cpu_mem=1200` ，让 PaddleOCR-json 在内存占用超过 1200MB 时执行自动内存清理。

- `--cpu_mem` 是 PaddleOCR-json v1.4.1 新增参数，v1.4.0或以前的版本无法使用。
- `--cpu_mem` 不建议低于 `1000` ，否则频繁清理内存，可能影响OCR效率。您也可以使用隔壁 [RapidOCR-json](https://github.com/hiroi-sora/RapidOCR-json) ，对低配置机器更友好。

另外，本项目默认不会在空闲时自动释放内存。假设当前占用了 1500MB ，那么接下来就算不进行OCR，引擎也会永远保持该内存占用，直到达到 `--cpu_mem` 设定的上界，自动执行清理。

如果你想让引擎在空闲时不要占用那么多内存，可行的方法有：

1. 外部重启引擎。使用一个程序/脚本来管理引擎的输入输出。如果持续一段时间没有活动，则kill掉引擎进程，重新启动一个。Umi-OCR就是这么做的。
2. 从引擎内部来清理内存。在分支 `release/1.4.0_autoclean` 里面是一个修改过的引擎。新增了一个参数 `--auto_memory_cleanup`，它会开启一条线程来检查引擎状态，然后在其闲置时释放内存。

> [!CAUTION]
> **无论是 `--auto_memory_cleanup` 还是 `--cpu_mem` 进行一次清理，内存占用仍会残留大约 300~600MB 。这是 Paddle Inference 推理库实例的占用下限。如果想空闲时接近0占用，只能使用方法1，外部重启引擎进程。**

更多细节请看这些Issue：[#43](https://github.com/hiroi-sora/PaddleOCR-json/issues/43)、[#90](https://github.com/hiroi-sora/PaddleOCR-json/issues/90)、[#135](https://github.com/hiroi-sora/PaddleOCR-json/issues/135)

> 如果你打算使用上面提到的方法2，请拉取并切换到 `release/1.4.0_autoclean` 分支：  
> ```sh
> git fetch origin release/1.4.0_autoclean
> git checkout -b release/1.4.0_autoclean origin/release/1.4.0_autoclean
> ```