#### 离线OCR组件 系列项目：
- **PaddleOCR-json**
- [RapidOCR-json](https://github.com/hiroi-sora/RapidOCR-json)

|                  | PaddleOCR-json                                  | RapidOCR-json        |
| ---------------- | ----------------------------------------------- | -------------------- |
| CPU要求          | CPU必须具有AVX指令集。不支持以下CPU：           | 无特殊要求 👍         |
|                  | 凌动Atom，安腾Itanium，赛扬Celeron，奔腾Pentium |                      |
| 推理加速库       | mkldnn 👍                                        | 无                   |
| 识别速度         | 快（启用mkldnn加速）👍                           | 中等                 |
|                  | 极慢（不启用mkldnn）                            |                      |
| 初始化耗时       | 约2s，慢                                        | 0.1s内，快 👍         |
| 组件体积（压缩） | 52MB                                            | 15MB 👍               |
| 组件体积（部署） | 250MB                                           | 30MB 👍               |
| CPU占用          | 高，榨干硬件性能                                | 较低，对低配机器友好 |
| 内存占用峰值     | >2000MB（启用mkldnn）                           | ~500MB 👍             |
|                  | ~600MB（不启用mkldnn）                          |                      |

---

# PaddleOCR-json

> 支持： **Win7 x64**、**Linux x64**、[Docker](cpp/README-docker.md)

这是一个基于 [PaddleOCR v2.6 C++](https://github.com/PaddlePaddle/PaddleOCR/tree/release/2.6) 的离线图片OCR文字识别程序，可快速让你的程序拥有OCR能力。它可以作为一个子进程被上层程序调用，也可以作为一个单独的进程通过TCP调用。本项目提供了Python等语言的API，你可以无视技术细节，通过两行代码使用它。

本项目旨在提供一个封装好的OCR引擎组件，使得没有C++编程基础的开发者也可以用别的语言来简单地调用OCR，享受到更快的运行效率、更便捷的打包&部署手段。

- **方便** ：部署方便，解压即用，无需安装和配置环境，无需联网。发布方便，可嵌入程序包也可作为外挂组件。
- **高速** ：基于 PPOCR C++ 版引擎，识别效率显著高于Python版本PPOCR及其他一些由Python编写的OCR引擎。
- **精准** ：附带PPOCR-v3识别库，对非常规字形（手写、艺术字、小字、杂乱背景等）也具有不错的识别率。
- **灵活** ：可以以多种方式指定OCR任务，支持识别本地图片路径、剪贴板图片、Base64编码的图片。

**应用：[Umi-OCR 批量图片转文字工具](https://github.com/hiroi-sora/Umi-OCR)**

## 兼容性

- 系统支持 Win7 x64 及以上。Linux的支持正在筹备。
- 若 Win7 报错`计算机中丢失 VCOMP140.DLL` ，请安装 [VC运行库](https://aka.ms/vs/17/release/vc_redist.x64.exe) 。
- CPU必须具有AVX指令集。常见的家用CPU一般都满足该条件。

    | AVX   | 支持的产品系列                                         | 不支持                                          |
    | ----- | ------------------------------------------------------ | ----------------------------------------------- |
    | Intel | 酷睿Core，至强Xeon                                     | 凌动Atom，安腾Itanium，赛扬Celeron，奔腾Pentium |
    | AMD   | 推土机架构及之后的产品，如锐龙Ryzen、速龙Athlon、FX 等 | K10架构及之前的产品                             |
- 如果需求在无AVX的CPU上使用OCR，可看看隔壁 [RapidOCR-json](https://github.com/hiroi-sora/RapidOCR-json) 。


## 准备工作

下载可执行文件包：

- Windows： [Release](https://github.com/hiroi-sora/PaddleOCR-json/releases/latest)
- Linux： [Gavin1937/Release](https://github.com/Gavin1937/PaddleOCR-json/releases/tag/v1.4.0-alpha.2)

### 简单试用

`PaddleOCR-json.exe -image_path="test.jpg"`

## 通过API调用

调用流程大体分为如下几步。不同API的具体接口可能有细微差别。

- 启动：启动并初始化引擎子进程。
- 工作：调用识图接口，获取返回值。目前支持识别 **本地图片文件** 、 **剪贴板中的图片** 、 **Base64编码的图片** 。
- 关闭：结束引擎进程，释放内存资源。

## API列表

`资源目录`下有更详细的使用说明及demo。

### 1. Python API

[资源目录](api/python)

<details>
<summary>使用示例</summary>

```python
from PPOCR_api import GetOcrApi

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
ocr = GetOcrApi("……\PaddleOCR-json.exe")

# 识别图片，传入图片路径
getObj = ocr.run(r'………\测试.png')
print(f'图片识别完毕，状态码：{getObj["code"]} 结果：\n{getObj["data"]}\n')
```

Python API 有丰富的附加模块：便于开发者调试观察的可视化模块；和[Umi-OCR](https://github.com/hiroi-sora/Umi-OCR)带来的文本块后处理（段落合并）技术。详细使用方法见 [资源目录](api/python)

</details>

### 2. Node.js API

[资源目录](api/node.js)

<details>
<summary>使用示例</summary>

```
npm install paddleocrjson
```

```js
const OCR = require('paddleocrjson');

// const OCR = require('paddleocrjson/es5'); // ES5

const ocr = new OCR('PaddleOCR-json.exe', [/* '-port=9985', '-addr=loopback' */], {
    cwd: './PaddleOCR-json',
}, false);

ocr.flush({ image_path: 'path/to/test/img' })
    .then((data) => console.log(data));
    .then(() => ocr.terminate());
```

</details>

### 3. PowerShell API

[资源目录](api/PowerShell)

### 4. Java API

[资源目录](https://github.com/jerrylususu/PaddleOCR-json-java-api)

### 5. .NET API

[资源目录](https://github.com/aki-0929/PaddleOCRJson.NET)


### 6. Rust API

[资源目录](https://github.com/OverflowCat/paddleocr)

### 7. Go API

[资源目录](https://github.com/doraemonkeys/paddleocr)

### 更多语言API

欢迎补充！请参考 [详细使用指南](docs/详细使用指南.md) 。


## 常用配置参数说明

| 键名称         | 默认值 | 值说明                                                                              |
| -------------- | ------ | ----------------------------------------------------------------------------------- |
| config_path    | ""     | 可以指定不同语言的配置文件路径，识别多国语言。[详情见下节](#语言库与切换识别语言)。 |
| models_path    | ""     | 可以指定语言库 `models` 文件夹的路径。[详情见下节](#语言库与切换识别语言)。         |
| cls            | false  | 启用cls方向分类，识别方向不是正朝上的图片。                                         |
| use_angle_cls  | false  | 启用方向分类，必须与cls值相同。                                                     |
| enable_mkldnn  | true   | 启用CPU推理加速，关掉可以减少内存占用，但会降低速度。                               |
| limit_side_len | 960    | 对图像边长进行限制，降低分辨率，加快速度。                                          |
|                |        | 如果对大图/长图的识别率低，可增大 limit_side_len 的值。                             |
|                |        | 建议为 32 & 48 的公倍数，如 960, 2880, 4320                                         |
| auto_memory_cleanup  | 1800   | 设置无OCR工作时自动清理内存来减少占用。                                        |
|                |        | 值 <= 0 时禁用功能。                                                               |
|                |        | 值 > 0 时将值作为时间（秒），每当无OCR工作一段时间后就清理一次内存。                   |
|                |        | 默认值 1800 秒（30分钟）。                                                         |

更多参数详见 [args.cpp](/cpp/src/args.cpp) 。（不支持其中GPU相关、表格识别相关的参数。-）

### 语言库与切换识别语言：

`v1.3`版本的Release压缩包中，默认附带了 `简中,繁中,英,日,韩,俄,德,法` 的语言库与配置文件，在 `models` 目录下。

`models` 目录中，每一个 `config_xxx.txt` 是一组语言配置文件（如英文是`congfig_en.txt`）。只需将这个文件的路径传入 `config_path` 参数，即可切换为对应的语言。以 Python API 为例：

```python
enginePath = "D:/Test/PaddleOCR_json.exe"  # 引擎路径
argument = {"config_path": "models/config_en.txt"}  # 指定使用英文库
ocr = GetOcrApi(enginePath, argument)
```

如果 config_path 留空，则 PaddleOCR-json 默认加载并使用简体中文识别库。

但是，当使用默认路径或单独设置 `config_path` 时，PaddleOCR-json可执行文件必须与语言库在同一目录下。比如：

```
.
├─ PaddleOCR-json.exe
└─ models
    ├─ ...
```

如果语言库在另外一个文件夹下，PaddleOCR-json就无法找到语言库。

在这种情况下，你可以使用 `models_path` 参数来设置语言库的位置。PaddleOCR-json会使用用户设置的语言库位置为基准来加载其他文件。

这样一来，即使 PaddleOCR-json 与语言库不在同一目录下也能正常使用。以 Python API 为例：

```python
enginePath = "D:/Test/PaddleOCR_json.exe"  # 引擎路径
modelsPath = "D:/Hello/models"             # 语言库路径路径
# 这里的参数顺序不影响结果
argument = {
  # 指定语言库位置
  "models_path": "D:/Hello/models",
  # 指定使用英文库
  "config_path": "D:/Hello/models/config_en.txt",
}
ocr = GetOcrApi(enginePath, argument)
```

#### 删除语言库：

若你希望删除吃灰的语言库文件以便减少软件体积，可以删除 `models` 目录中含有对应语言前缀和 **rec_infer** 后缀的文件夹。比如你希望删除日语`japan`相关的库，只需删除该文件夹：  
`japan_PP-OCRv3_rec_infer`

一组语言的rec库大约占用10MB空间（未压缩）。若删除到仅剩1组语言，可以节省约60MB空间。

请不要删除cls_infer及det_infer后缀的文件夹，这是所有语言公用的检测/方向分类库。


## 返回值说明

通过API调用一次OCR，无论成功与否，都会返回一个字典。

字典中，根含两个元素：状态码`code`和内容`data`。

状态码`code`为整数，每种状态码对应一种情况：

##### `100` 识别到文字

- data内容为数组。数组每一项为字典，含三个元素：
  - `text` ：文本内容，字符串。
  - `box` ：文本包围盒，长度为4的数组，分别为左上角、右上角、右下角、左下角的`[x,y]`。整数。
  - `score` ：识别置信度，浮点数。
- 例：
  ```
    {'code':100,'data':[{'box':[[13,5],[161,5],[161,27],[13,27]],'score':0.9996442794799805,'text':'飞舞的因果交流'}]}
  ```

##### `101` 未识别到文字

- data为字符串：`No text found in image. Path:"图片路径"`
- 例：```{'code':101,'data':'No text found in image. Path: "D:\\空白.png"'}```
- 这是正常现象，识别没有文字的空白图片时会出现这种结果。

##### `200` 图片路径不存在

- data：`Image path dose not exist. Path:"图片路径".`
- 例：`{'code':200,'data':'Image path dose not exist. Path: "D:\\不存在.png"'}`
- 注意，在系统未开启utf-8支持（`使用 Unicode UTF-8 提供全球语言支持"`）时，不能读入含emoji等特殊字符的路径（如`😀.png`）。但一般的中文及其他 Unicode 字符路径是没问题的，不受系统区域及默认编码影响。

##### `201` 图片路径string无法转换到wstring

- data：`Image path failed to convert to utf-16 wstring. Path: "图片路径".`
- 使用API时，理论上不会报这个错。
- 开发API时，若传入字符串的编码不合法，有可能报这个错。

##### `202` 图片路径存在，但无法打开文件

- data：`Image open failed. Path: "图片路径".`
- 可能由系统权限等原因引起。

##### `203` 图片打开成功，但读取到的内容无法被opencv解码

- data：`Image decode failed. Path: "图片路径".`
- 注意，引擎不以文件后缀来区分各种图片，而是对存在的路径，均读入字节尝试解码。若传入的文件路径不是图片，或图片已损坏，则会报这个错。
- 反之，将正常图片的后缀改为别的（如`.png`改成`.jpg或.exe`），也可以被正常识别。

<details>
<summary>
<strong>剪贴板相关接口已弃用，不建议使用</strong>
</summary>

##### `210` 剪贴板打开失败

- data：`Clipboard open failed.`
- 可能由别的程序正在占用剪贴板等原因引起。

##### `211` 剪贴板为空

- data：`Clipboard is empty.`

##### `212` 剪贴板的格式不支持

- data：`Clipboard format is not valid.`
- 引擎只能识别剪贴板中的位图或文件。若不是这两种格式（如复制了一段文本），则会报这个错。

##### `213` 剪贴板获取内容句柄失败

- data：`Getting clipboard data handle failed.`
- 可能由别的程序正在占用剪贴板等原因引起。

##### `214` 剪贴板查询到的文件的数量不为1

- data：`Clipboard number of query files is not valid. Number: 文件数量`
- 只允许一次复制一个文件。一次复制多个文件再调用OCR会得到此报错。

##### `215` 剪贴板检索图形对象信息失败

- data：`Clipboard get bitmap object failed.`
- 剪贴板中是位图，但获取位图信息失败。可能由别的程序正在占用剪贴板等原因引起。

##### `216` 剪贴板获取位图数据失败

- data：`Getting clipboard bitmap bits failed.`
- 剪贴板中是位图，获取位图信息成功，但读入缓冲区失败。可能由别的程序正在占用剪贴板等原因引起。

##### `217` 剪贴板中位图的通道数不支持

- data：`Clipboard number of image channels is not valid. Number: 通道数`
- 引擎只允许读入通道为1（黑白）、3（RGB）、4（RGBA）的图片。位图通道数不是1、3或4，会报这个错。

</details>

##### `300` base64字符串解析为string失败

- data：`Base64 decode failed.`
- 传入非法Base64字符串引起。（注意，传入Base64信息不应带有`data:image/jpg;base64,`前缀。）

##### `301` base64字符串解析成功，但读取到的内容无法被opencv解码

- data：`Base64 data imdecode failed.`

##### `400` json对象 转字符串失败

- data：`Json dump failed.CODE_ERR_JSON_DUMP`
- 输入异常：传入非法json字符串，或者字符串含非utf-8编码字符导致无法解析引起。

##### `401` json字符串 转对象失败

- data：`Json dump failed.CODE_ERR_JSON_DUMP`
- 输出异常：输出时OCR结果无法被编码为json字符串。

##### `402` json对象 解析某个键时失败

- data：`Json parse key 键名 failed.`
- 比错误码`400`更精准的提示。如果发生异常，程序优先报`402`，无法处理才报`400`。

##### `403` 未发现有效任务

- data：`No valid tasks.`
- 本次传入的指令中不含有效任务。


### [详细使用指南](docs/详细使用指南.md)

👆当你需要修改或开发新API时欢迎参考。


### 项目构建指南

- [Windows 平台](cpp/README.md)
- [Linux 平台](cpp/README-linux.md)
- [Docker 部署](cpp/README-docker.md)
- [移植指南](cpp/docs/移植指南.md) （需要移植项目到不同平台时可供参考）

### 感谢

本项目中使用了 [ReneNyffenegger/cpp-base64](https://github.com/ReneNyffenegger/cpp-base64) ：
> “base64 encoding and decoding with c++”

本项目中使用了 [nlohmann/json](https://github.com/nlohmann/json) ：
> “JSON for Modern C++”

感谢 [PaddlePaddle/PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) ，没有它就没有本项目：
> “Awesome multilingual OCR toolkits based on PaddlePaddle”

感谢各位为本项目开发API及贡献代码的朋友！

## 更新日志

版本号链接可前往对应备份分支。

#### v1.3.1 `2023.10.10` 
- 兼容 Win7 x64 。

#### [v1.3.0](https://github.com/hiroi-sora/PaddleOCR-json/tree/release/1.3.0) `2023.6.19` 
- 修复了一些BUG。

#### v1.3.0 Alpha `2023.5.26` 
- 重构代码，条理更清晰，易于移植。
- 新功能：Base64传图片。
- 新功能：套接字服务器模式。

#### [v1.2.1](https://github.com/hiroi-sora/PaddleOCR-json/tree/backups/1.2.1/new_builds) `2022.9.28` 
- 修复了一些BUG。
- 解决非中文windows难以读取中文路径的问题，拥抱utf-8，彻底摆脱对gbk等区域性编码的依赖。
- 新功能：直接读取并识别剪贴板内存中的图片。
- 错误代码和提示更详细。

#### [v1.2.0](https://github.com/hiroi-sora/PaddleOCR-json/tree/release/1.2.0) `2022.8.29` 
- 修复了一些BUG。
- 增强了面对不合法编码时的健壮性。
- 默认开启mkldnn加速。
- 新功能：json输入及热更新。

#### v1.2.0 Beta `2022.8.26` 
- 重构整个工程，核心代码同步PaddleOCR 2.6。
- 对v3版识别库的支持更好。
- 新功能：启动参数。
- 新功能：ascii转义。（感谢 @AutumnSun1996 的提议 [issue #4](https://github.com/hiroi-sora/PaddleOCR-json/issues/4) ）

#### [v1.1.1](https://github.com/hiroi-sora/PaddleOCR-json/tree/release/1.1.1) `2022.4.16` 
- 修正了漏洞：当`文本检测`识别到区域但`文本识别`未在区域中检测到文字时，可能输出不相符的包围盒。

#### v1.1.0 `2022.4.2` 
- 修改了json输出格式，改为状态码+内容，便于调用方判断。

#### v1.0 `2022.3.28`
