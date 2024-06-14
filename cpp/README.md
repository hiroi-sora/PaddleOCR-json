# PaddleOCR-json V1.3 Windows 构建指南

本文档帮助如何在Windows上编译 PaddleOCR-json V1.3 （对应PPOCR v2.6）。面向小白，用的最简单的步骤。大佬可酌情调整。

本文参考了 PPOCR官方的[编译指南](https://github.com/PaddlePaddle/PaddleOCR/blob/release/2.6/deploy/cpp_infer/docs/windows_vs2019_build.md#12-%E4%B8%8B%E8%BD%BD-paddlepaddle-c-%E9%A2%84%E6%B5%8B%E5%BA%93%E5%92%8C-opencv) ，但建议以本文为准。

相关文档：
- [Linux 构建指南](./README-linux.md)
- [Docker 部署](#使用-docker-部署)
- 其他平台 [移植指南](docs/移植指南.md)


## 1. 前期准备

资源链接后面的(括弧里是版本)，请看清楚。

### 1.1 需要安装的工具：

- [Visual Studio 2022](https://learn.microsoft.com/zh-cn/visualstudio/releases/2022/release-notes) (2022或2019均可，Community)
- [Cmake](https://cmake.org/download/) (Windows x64 Installer)
- [Git](https://git-scm.com/download/win) (64-bit Git for Windows Setup)

### 1.2 需要下载的资源：

- [paddle_inference](https://paddleinference.paddlepaddle.org.cn/user_guides/download_lib.html#windows) (Windows, 2.3.2, C++预测库, cpu_avx_mkl)
- [Opencv](https://github.com/opencv/opencv/releases) (windows.exe)
- [模型库](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/models%2Fv1.3) (models.zip)

如果是 Linux / Mac 平台，则paddle_inference和Opencv需选择对应平台的资源。模型库通用，无需改。

### 1.3 放置资源

1. clone 本仓库。在 `PaddleOCR-json/cpp` 下新建一个文件夹 `.source` 来存放外部资源。（前面加点是为了按文件名排列更顺眼）
2. 将下载好的 `models.zip` 、 `paddle_inference` 和 `Opencv` 解压进`.source`。
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
然后，点击左下角第一个按钮Configure，第一次点它会弹出提示框进行Visual Studio配置，选择你的Visual Studio版本即可，目标平台选择x64。然后点击finish按钮即开始自动执行配置。

Where is the source code: `……/PaddleOCR-json/cpp`

Where to build the binaries: `……/PaddleOCR-json/cpp/build`

![](docs/imgs/b1.png)

执行完会报错，很正常，点OK。
![](docs/imgs/b2.png)

2. 填写三项配置：

OPENCV_DIR 和 OpenCV_DIR:  
`……/PaddleOCR-json/cpp/.source/opencv/build/x64/vc16/lib`

PADDLE_LIB:  
`……/PaddleOCR-json/cpp/.source/paddle_inference_cpu_avx_mkl`

下面 `WITH_GPU` 确保**不要**勾选。

其他项就不要动了！

![](docs/imgs/b3.png)

点击左下角 **第一个按钮Configure** 应用配置，等待几秒，看到输出 `Configuring done` 即可。

点击左下角 **第二个按钮Generate** 即可生成Visual Studio 项目的sln文件。看到输出 `Generating done` 即可。那么，你会看到 `PaddleOCR-json/cpp/build` 下生成了 `PaddleOCR-json.sln` 及一堆文件。

#### CMake构建参数

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

以下是一些PaddleOCR-json功能相关参数。

| 参数名                   | 描述                               |
| ------------------------ | ---------------------------------- |
| `ENABLE_CLIPBOARD`       | 启用剪贴板功能。默认关闭。         |
| `ENABLE_REMOTE_EXIT`     | 启用远程关停服务器命令。默认开启。 |
| `ENABLE_JSON_IMAGE_PATH` | 启用json命令image_path。默认开启。 |

> [!NOTE]
> * `ENABLE_REMOTE_EXIT`: 这个参数控制着 “传入 `exit` 关停服务器” 的功能。
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

2. 调整项目字符集。在解决方案管理器的PaddleOCR-json上，右键→属性→高级→`字符集`改为`使用Unicode字符集`。

![](docs/imgs/b9.png)

3. 按F5编译。如果输出`生成：成功2个，失败0个……`，弹窗`无法启动程序：……系统找不到指定的文件` 是正常的。但你应该能在 `build/bin/Release` 下找到生成的 `PaddleOCR-json.exe` 。请跳到第6步。

4. 如果编译时，报了大量的语法错误，如：
   ```
   C2447 "{"缺少函数标题
   C2059 语法错误："if"
   C2143 语法错误：缺少";"
   …………等等
   ```
   那么可能是源代码文件的换行符编码问题。解决方法一：通过`git clone`下载本仓库代码，而不要直接在Github下载zip文件包。解决方法二：批量将所有`.h`和`.cpp`文件的 [换行符转换为CRLF](https://www.bing.com/search?q=%E6%89%B9%E9%87%8F%E8%BD%AC%E6%8D%A2+LF+%E5%92%8C+CRLF) 。

5. 拷贝必要的运行库。在 `.source` 中的 `paddle_inference_cpu_avx_mkl` 及 `opencv` 目录中。`paddle_inference_cpu_avx_mkl` 里面的运行库会在项目构建时自带被拷贝到 `build/bin/Release` 文件夹下。你需要拷贝 `opencv` 的运行库到 `build/bin/Release` 文件夹下。

- `opencv/build/x64/vc16/bin/opencv_world***.dll`

> 当然，你也可以直接将 `opencv` 的运行库放到Windows的 `PATH` 环境变量中。参考[这篇文档](https://cloud.baidu.com/article/3297806)，把路径 `opencv/build/x64/vc16/bin/` 加入 `PATH`。这样就不需要拷贝 `opencv` 运行库了。

6. 拷贝模型库。将 `.source` 中的 `models` 整个拷贝到 `build/bin/Release` 文件夹下。

7. 在`build/bin/Release`下，Shift+右键，在此处打开终端（或PowerShell），输入 `./PaddleOCR-json.exe` 。如果输出下列文字，就正常。

```
OCR anonymous pipe mode.
OCR init completed.
```

8. 回到 Visual Studio 中，再进行一些配置。  
- 首先在 `ALL BUILD` 上，右键→属性→常规，**输出目录** 原本是 `$(SolutionDir)$(Platform)/$(Configuration)/` ，现在改成exe生成的目录，即为 `$(ProjectDir)/Release` 。  
- **目标文件名** 改成 `PaddleOCR-json` 。  
- 修改工作目录：调试→**工作目录**，原来是`$(ProjectDir)` ，将它改为 `$(ProjectDir)/Release`。

![](docs/imgs/b6.png)
![](docs/imgs/b7.png)

9. 尝试按F5重新编译。如果成功生成，并且有一个命令行窗口一闪而过，那就说明配置正确了。

如果你需要移植其他平台，可以参考文档 [移植指南](docs/移植指南.md)

## 4. 安装

你可以使用CMake来安装PaddleOCR-json。在`cpp`下，Shift+右键，在此处打开终端（或PowerShell），运行下面这条命令。

```sh
cmake --install build
```

CMake会将 `build` 文件夹下的可执行文件和运行库给安装到 `build/install/bin` 文件夹下。CMake无法在Windows下把软件安装到系统文件夹中，不过你可以将文件夹 `cpp/build/install/bin` 添加到Windows的 `PATH` 环境变量中。参考[这篇文档](https://cloud.baidu.com/article/3297806)。

如果你希望安装到指定位置，你可以为上面这条命令加上参数 `--prefix /安装路径/` 来指定一个安装路径。比如 `--prefix build/install` 会将所有的文件都安装到 `build/install` 文件夹下。


## 使用 Docker 部署

你可以将本项目部署到Docker容器里，然后通过套接字来连接使用。

部署步骤：

1. [安装Docker](https://yeasy.gitbook.io/docker_practice/install)
2. clone 本仓库，**然后在 `cpp` 文件夹下打开一个终端（或PowerShell）**。
3. 接着使用Docker来构建镜像

```sh
docker build -t paddleocr-json .
```

> [!TIP]
> 你可以使用docker `--build-arg` 参数来开/关镜像的一些功能。
> * `ENABLE_REMOTE_EXIT`: 控制是否启用远程关停服务器命令，[详情如下](#cmake构建参数)。默认开启。
> * `ENABLE_JSON_IMAGE_PATH`: 控制是否启用json命令image_path，[详情如下](#cmake构建参数)。默认关闭。
> 
> 比如下面这条修改过的命令。它会禁用远程关停服务器与image_path json命令，这样一来这个镜像就变成了一个纯服务器镜像。用户无法轻易的关停服务器或令服务器读取容器内的文件。
> ```sh
> docker build -t paddleocr-json \
>     --build-arg "ENABLE_REMOTE_EXIT=OFF" \
>     --build-arg "ENABLE_JSON_IMAGE_PATH=OFF" .
> ```
> * 最后那个 `.` 必须放在整段命令的末尾，它指定了当前文件夹为docker镜像构建的基础文件夹。
> [了解更多关于 `--build-arg` 的信息](https://yeasy.gitbook.io/docker_practice/image/dockerfile/arg)

> [!NOTE]
> 可能出现的错误：
> * 如果在镜像构建的过程中出现了类似下面这样的错误。这大概率是网络问题，建议更换docker镜像源或者更换dns。
>
> ```
> Ign:516 http://deb.debian.org/debian stable/main amd64 va-driver-all amd64 2.17.0-1
> Ign:517 http://deb.debian.org/debian stable/main amd64 vdpau-driver-all amd64 1.5-2
> Ign:518 http://deb.debian.org/debian stable/main amd64 xauth amd64 1:1.1.2-1
> Ign:519 http://deb.debian.org/debian stable/main amd64 xdg-user-dirs amd64 0.18-1
> Ign:520 http://deb.debian.org/debian stable/main amd64 zip amd64 3.0-13
> Err:402 http://deb.debian.org/debian stable/main amd64 libmunge2 amd64 0.5.15-2
> Could not connect to deb.debian.org:80 (146.75.94.132), connection timed out [IP: 146.75.94.132 80]
> Err:403 http://deb.debian.org/debian stable/main amd64 libtbbmalloc2 amd64 2021.8.0-2
> Unable to connect to deb.debian.org:80: [IP: 146.75.94.132 80]
> Err:404 http://deb.debian.org/debian stable/main amd64 libtbbbind-2-5 amd64 2021.8.0-2
> Unable to connect to deb.debian.org:80: [IP: 146.75.94.132 80]
> Err:405 http://deb.debian.org/debian stable/main amd64 libtbb12 amd64 2021.8.0-2
> ```

4. 接着就可以部署了

```sh
docker run -d \
   --name paddleocr-json \
   -p 3746:3746 \
   paddleocr-json
```

* 这里我们使用参数 `-d` 来以后台模式运行容器。
* 使用参数 `--name` 来命名Docker容器。
* 使用参数 `-p` 来暴露容器端口 `3746` 到本地端口 `3746`。容器在运行时会默认将套接字服务器开在容器端口 `3746` 上。
* 最后使用我们刚刚构建的镜像 `paddleocr-json` 来创建容器。

> [!TIP]
> * 你可以在这条docker命令的末尾加上各种PaddleOCR-json参数来修改服务器。更多配置参数请参考[简单试用](../README.md#简单试用)和[常用配置参数说明](../README.md#常用配置参数说明)
> * 并且，PaddleOCR-json已经被安装到了容器系统里，你可以在容器里直接用 `PaddleOCR-json` 来运行它。当然，你需要模型库。
> * 容器自带一套[模型库](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/models%2Fv1.3)，存放在 `/app/models` 路径下。如果你希望使用自己的模型库，你可以[使用Docker挂载一个数据卷到容器里](https://yeasy.gitbook.io/docker_practice/data_management/volume#qi-dong-yi-ge-gua-zai-shu-ju-juan-de-rong-qi)，然后使用参数 `-models_path` 来指定新的模型库路径。
