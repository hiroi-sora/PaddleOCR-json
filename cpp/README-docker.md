# PaddleOCR-json V1.4 Docker 部署指南

你可以将本项目部署到Docker容器里，然后通过API或套接字来连接使用。

部署步骤：

1. [安装Docker](https://yeasy.gitbook.io/docker_practice/install)
2. clone 本仓库，**然后在 `cpp` 文件夹下打开一个终端（或PowerShell）**。
3. 接着我们使用Docker来构建镜像

```sh
docker build -t paddleocr-json .
```

> [!NOTE]
> 镜像构建默认会开启远程关停服务器功能，并且禁用从路径读取图片的功能。从剪贴板读取图片的功能默认禁用，而且它在Linux下也不存在。

> [!TIP]
> 你可以使用docker `--build-arg` 参数来开/关镜像的一些功能。
> * `ENABLE_REMOTE_EXIT`: 控制是否启用远程关停引擎进程命令，[详情请看这里](README.md#cmake构建参数)。默认开启。
> * `ENABLE_JSON_IMAGE_PATH`: 控制是否启用json命令image_path，[详情请看这里](README.md#cmake构建参数)。默认关闭。
> 
> 比如下面这条修改过的命令。它会禁用远程关停服务器与image_path json命令，这样一来这个镜像就变成了一个纯服务器镜像。用户无法轻易的关停服务器或令服务器读取容器内的文件。
> ```sh
> docker build -t paddleocr-json \
>     --build-arg "ENABLE_REMOTE_EXIT=OFF" \
>     --build-arg "ENABLE_JSON_IMAGE_PATH=OFF" \
>     .
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
> * 你可以在上面这条docker命令的末尾加上各种PaddleOCR-json参数来修改服务器。更多配置参数请参考[简单试用](../README.md#简单试用)和[常用配置参数说明](../README.md#常用配置参数说明)
> * 并且，PaddleOCR-json已经被安装到了容器系统里，你可以在容器里直接用 `PaddleOCR-json` 来运行它。当然，你需要模型库。
> * 容器自带一套[模型库](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/models%2Fv1.3)，存放在 `/app/models` 路径下。如果你希望使用自己的模型库，你可以[使用Docker挂载一个数据卷到容器里](https://yeasy.gitbook.io/docker_practice/data_management/volume#qi-dong-yi-ge-gua-zai-shu-ju-juan-de-rong-qi)，然后使用参数 `-models_path` 来指定新的模型库路径。

## 将PaddleOCR-json Docker镜像集成到你的Dockerfile里面

你可以在完成PaddleOCR-json的Docker镜像构建之后将其集成在其他的Dockerfile里。

假设你需要在一个容器里运行PaddleOCR-json和另一个进程，我们先写一个简单的bash脚本来启动多个进程到后台。

#### run.sh

```sh
#!/bin/bash

# 启动PaddleOCR-json到后台，监听 127.0.0.1:1234，这样你的进程就可以通过这个端口来与PaddleOCR-json通信
PaddleOCR-json -models_path /app/models -addr loopback -port 1234 &

# 启动你的进程到后台
./my_process &

# 等待任意一个进程退出
wait -n

# 以第一个退出的进程的退出状态来退出脚本
exit $?
```

接下来我们就可以写一个新的Dockerfile了

#### Dockerfile

```dockerfile
# 以我们构建的Docker镜像为基础
FROM paddleocr-json:latest

# 运行其他命令
RUN ...

# 复制其他文件
COPY ....

# 复制我们刚刚写的脚本
COPY run.sh run.sh

# 启动PaddleOCR-json引擎进程
# 这里需要用 ENTRYPOINT 来覆盖掉 paddleocr-json 镜像里的 ENTRYPOINT
# 用 CMD 会出错
ENTRYPOINT ["./run.sh"]
```

之后直接构建镜像再运行就行了

```sh
docker build -t myimage .
```

```sh
docker run -d --name mycontainer myimage
```

更多例子请看：
* [官方文档](https://docs.docker.com/config/containers/multi-service_container/)
* [腾讯云文章](https://cloud.tencent.com/developer/article/1683445)

## 通过API调用OCR

我们提供了 Python、Java 等语言的API，详见 [README-通过API调用](../README.md/#通过API调用) 。您可以通过这些API的 **套接字模式** 来调用Docker中的OCR服务，向API接口传入容器的IP和暴露端口即可。

## 其他问题

### [关于内存泄漏 / 长期高内存占用](./README.md#关于内存泄漏--长期高内存占用)

如果你打算使用文档中提到的方法2，由于docker镜像在构建时就会自动构建编译整个项目，所以您只需要重新clone一遍本仓库并重新构建一遍docker镜像就行了。
