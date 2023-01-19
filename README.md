# PaddleOCR-json 图片转文字程序 v1.2.1

这是一个基于 [PaddleOCR v2.6 C++](https://github.com/PaddlePaddle/PaddleOCR/tree/release/2.6) 的离线图片OCR文字识别程序。通过管道等方式输入本地图片路径，输出识别结果json字符串。适用于Win10系统。

- **方便** ：解压即用，无需安装和配置环境，无需联网。引入API，两行代码调用OCR。（未提供API的语言，可参考文档通过管道调用OCR）
- **高速** ：基于 PPOCR C++ 版引擎，识别效率显著高于Python版本PPOCR及其他一些由Python编写的OCR引擎。
- **精准** ：附带PPOCR-v3识别库，对非常规字形（手写、艺术字、小字、杂乱背景等）也具有不错的识别率。

![](https://tupian.li/images/2022/09/23/1699a5d8765090de0.png)

**关联项目：[Umi-OCR 批量图片转文字工具](https://github.com/hiroi-sora/Umi-OCR)**

### `v1.2.1` 版主要新增功能：

- 可读取剪贴板中的图片。
- 优化文件路径解析方式，修复了非中文Windows系统下可能无法读取中文路径的Bug。

### `v1.2.1` 版主要改动：

- 修改返回结果中包围盒的格式，使之更类似别的OCR接口的格式。从原来的
    ```
    "box": [1x, 1y, 2x, 2y, 3x, 3y, 4x, 4y]
    ```
    修改为
    ```
    "box": [[1x, 1y], [2x, 2y], [3x, 3y], [4x, 4y]]
    ```
- 错误标志码和错误信息作了少量修改，更详尽。
- 切换关闭debug模式，详见 [详细使用指南](docs/详细使用指南.md) 。

## 兼容性

- 系统支持 Win10 x64 。
- 不建议使用 Win7 ，识别引擎很可能无法运行。如果想尝试，win7 x64 sp1 打满系统升级补丁+安装vc运行库后有**小概率**能跑起来。
- CPU必须具有AVX指令集。常见的家用CPU一般都满足该条件。

    | AVX   | 支持的产品系列                                         | 不支持                | 存疑                     |
    | ----- | ------------------------------------------------------ | --------------------- | ------------------------ |
    | Intel | 酷睿Core，至强Xeon                                     | 凌动Atom，安腾Itanium | 赛扬Celeron，奔腾Pentium |
    | AMD   | 推土机架构及之后的产品，如锐龙Ryzen、速龙Athlon、FX 等 | K10架构及之前的产品   |                          |


## 准备工作

下载 [PaddleOCR-json v1.2.1](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/v1.2.1) 并解压，即可。

### 简单试用

双击打开 `PaddleOCR_json.exe` 。等程序初始化完毕输出`OCR init completed.`后，直接输入图片路径，回车。（此时不允许输入中文路径，通过API调用时可以。）

## 通过API调用

调用流程大体分为如下几步。不同API的具体接口可能有细微差别。

- 启动：传入引擎exe路径，启动并初始化引擎子进程。
- 工作：调用识图接口，获取返回值。目前支持识别 **本地图片文件** 和 **剪贴板中的图片** 。
- 关闭：结束引擎进程，释放内存资源。

可以传入各种参数来调整OCR的工作方式。

- 启动时：传入配置参数或配置文件路径。
- 工作中：通过 **热更新** 修改部分参数。

识图接口可执行两种任务：

- 本地文件任务：传入本地图片文件的路径。
- 剪贴板任务：调用API封装好的接口，或直接传入字符串`clipboard`。

关于剪贴板任务：目前支持识别剪贴板中的 **位图** （系统截屏、浏览器复制图片、微信复制图片）或 **文件句柄** （文件管理器选中图片复制）。

关于结果可视化：可在图片上绘制文本包围盒，保存到本地。


## API列表

`资源目录`下有更详细的使用说明及demo。

### 1. Python API

[资源目录](api/python)

<details>
<summary>使用示例</summary>

```python
from PPOCR_api import PPOCR

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
ocr = PPOCR('D:\…………\PaddleOCR-json\PaddleOCR_json.exe')

# 识别图片，传入图片路径
getObj = ocr.run(r'………\测试.png')
print(f'图片识别完毕，状态码：{getObj["code"]} 结果：\n{getObj["data"]}\n')

ocr.stop()  # 结束引擎子进程
```

Python API 有更丰富的附加模块：便于开发者调试观察的可视化模块；和[Umi-OCR](https://github.com/hiroi-sora/Umi-OCR)下放的文本块后处理（段落合并）技术。详细使用方法见 [资源目录](api/python)

</details>

### 2. PowerShell API

> 由[jiangzian04121735/PaddleOCR-json](https://github.com/jiangzian04121735/PaddleOCR-json)协助

[资源目录](api/powershell)

<details>
<summary>使用示例</summary>

```PowerShell
Import-Module -Force D:\…………\PPOCR_api.ps1

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
$ocr = [PPOCR]::new("D:\…………\PaddleOCR-json\PaddleOCR_json.exe")

# 识别图片，传入图片路径
$imgPath = "………\test.png" 
$getObj = $ocr.run($imgPath)
Write-Host "图片识别完毕，状态码：$($getObj.code) 结果：`n$($getObj.data | Out-String)`n"

$ocr.del()  # 结束子进程。
Write-Host "程序结束。"
```

</details>


### 3. Node.js API

> 由[PunchlY/PaddleOCR-json-node-api](https://github.com/PunchlY/PaddleOCR-json-node-api)贡献

[资源目录](api/node.js)

<details>
<summary>使用示例</summary>

```js
const OCR = require('./OCR');
const ocr = new OCR('PaddleOCR_json.exe', [], {
    cwd: './PaddleOCR-json',
}, false);

ocr.postMessage({ image_dir: 'path/to/test/img' })
    .then((data) => console.log(data));
    .then(() => ocr.terminate());
```

</details>

### 4. Java API

> 由[jerrylususu/PaddleOCR-json-java-api](https://github.com/jerrylususu/PaddleOCR-json-java-api)贡献

[资源目录](api/java)

<details>
<summary>使用示例</summary>

```java
// paddleocr_json 的可执行文件所在路径
String exePath = "path/to/executable";

// 可选的配置项
Map<String, Object> arguments = new HashMap<>();
// arguments.put("use_angle_cls", true);

// 初始化 OCR
try (Ocr ocr = new Ocr(new File(exePath), arguments)) {

    // 对一张图片进行 OCR
    OcrResponse resp = ocr.runOcr(new File("path/to/img"));

    // 读取结果
    if (resp.code == OcrCode.OK) {
        for (OcrEntry entry : resp.data) {
            System.out.println(entry.text);
        }
    } else {
        System.out.println("error: code=" + resp.code + " msg=" + resp.msg);
    }
} catch (IOException e) {
    e.printStackTrace();
}
```

</details>

### 5. Rust API

> 由[OverflowCat/paddleocr](https://github.com/OverflowCat/paddleocr)贡献

[资源目录](api/rust)

<details>
<summary>使用示例</summary>

```rust
fn main() {
    let mut p = paddleocr::Ppocr::new(std::path::PathBuf::from(
        "C:/.../PaddleOCR_json.exe", // PaddleOCR_json.exe 的路径
    ))
    .unwrap(); // 会检测是否出现 `OCR init completed.`，`Ok(x)` 说明初始化成功

    let now = std::time::Instant::now(); // 开始计算所需时间
    {
        // OCR 文件
        println!("{}", p.ocr("C:/.../test1.png").unwrap());
        println!("{}", p.ocr("C:/.../test2.png").unwrap());
        println!("{}", p.ocr("C:/.../test3.png").unwrap());
        println!("{}", p.ocr("C:/.../test4.png").unwrap());
        println!("{}", p.ocr("C:/.../test5.png").unwrap());

        // OCR 当前剪贴板
        println!("{}", p.ocr_clipboard().unwrap());
    }
    println!("Elapsed: {:.2?}", now.elapsed());

    // `struct Ppocr` 会自动在 `Drop` 时结束进程
}
```

</details>


### 更多语言API

欢迎补充！请参考 [详细使用指南](docs/详细使用指南.md) 。


## 返回值说明

通过API调用一次OCR，无论成功与否，都会返回一个字典。

字典中，根含两个元素：状态码`code`和内容`data`。在设置了热更新的回合，还会含有额外元素：更新日志`hotUpdate`。

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

- data为字符串：`Image path dose not exist. Path:"图片路径".`
- 例：`{'code':200,'data':'Image path dose not exist. Path: "D:\\不存在.png"'}`
- 注意，在系统未开启utf-8支持（`使用 Unicode UTF-8 提供全球语言支持"`）时，不能读入含emoji等特殊字符的路径（如`😀.png`）。但一般的中文及其他 Unicode 字符路径是没问题的，不受系统区域及默认编码影响。

##### `201` 图片路径string无法转换到wstring

- data为字符串：`Image path failed to convert to utf-16 wstring. Path: "图片路径".`
- 使用API时，理论上不会报这个错。
- 开发API时，若传入字符串的编码不合法，有可能报这个错。

##### `202` 图片路径存在，但无法打开文件

- data为字符串：`Image open failed. Path: "图片路径".`
- 可能由系统权限等原因引起。

##### `203` 图片打开成功，但读取到的内容无法被opencv解码

- data为字符串：`Image decode failed. Path: "图片路径".`
- 注意，引擎不以文件后缀来区分各种图片，而是对存在的路径，均读入字节尝试解码。若传入的文件路径不是图片，或图片已损坏，则会报这个错。
- 反之，将正常图片的后缀改为别的（如`.png`改成`.jpg或.exe`），也可以被正常识别。

##### `210` 剪贴板打开失败

- data为字符串：`Clipboard open failed.`
- 可能由别的程序正在占用剪贴板等原因引起。

##### `211` 剪贴板为空

- data为字符串：`Clipboard is empty.`

##### `212` 剪贴板的格式不支持

- data为字符串：`Clipboard format is not valid.`
- 引擎只能识别剪贴板中的位图或文件。若不是这两种格式（如复制了一段文本），则会报这个错。

##### `213` 剪贴板获取内容句柄失败

- data为字符串：`Getting clipboard data handle failed.`
- 可能由别的程序正在占用剪贴板等原因引起。

##### `214` 剪贴板查询到的文件的数量不为1

- data为字符串：`Clipboard number of query files is not valid. Number: 文件数量`
- 只允许一次复制一个文件。一次复制多个文件再调用OCR会得到此报错。

##### `215` 剪贴板检索图形对象信息失败

- data为字符串：`Clipboard get bitmap object failed.`
- 剪贴板中是位图，但获取位图信息失败。可能由别的程序正在占用剪贴板等原因引起。

##### `216` 剪贴板获取位图数据失败

- data为字符串：`Getting clipboard bitmap bits failed.`
- 剪贴板中是位图，获取位图信息成功，但读入缓冲区失败。可能由别的程序正在占用剪贴板等原因引起。

##### `217` 剪贴板中位图的通道数不支持

- data为字符串：`Clipboard number of image channels is not valid. Number: 通道数`
- 引擎只允许读入通道为1（黑白）、3（RGB）、4（RGBA）的图片。位图通道数不是1、3或4，会报这个错。

##### `299` 未知异常

- data为字符串：`An unknown error has occurred.`
- 请提issue。



##### hotUpdate元素

- `hotUpdate` 元素仅在设置了热更新的回合出现，与识别码`code`没有关联。其内容为记录热更新日志的字符串。
- 例：
    ```
    {"code":200,"data":"Image path not exist. Path:\"\"","hotUpdate":"det_model_dir set to ch_PP-OCRv2_det_infer. limit_side_len set to 961. rec_img_h set to 32. "}
    ```
- 可以在传入图片路径的同时热更新，引擎会先执行热更新再执行本轮OCR。也可以在单独的回合里执行热更新，图片路径为空即可，无视code==200。

## 配置参数说明

#### 静态参数

只能在启动引擎时注入，不能中途热更新。

| 键名称             | 值说明                                   | 默认值        |
| ------------------ | ---------------------------------------- | ------------- |
| det_model_dir      | det库路径                                | 必填          |
| cls_model_dir      | cls库路径                                | 必填          |
| rec_model_dir      | rec库路径                                | 必填          |
| rec_char_dict_path | rec字典路径                              | 必填          |
| rec_img_h          | v3模型填48，v2填32                       | 48            |
| det                | 启用det文本检测                          | true          |
| cls                | 启用cls方向分类，与use_angle_cls同时使用 | false         |
| use_angle_cls      | 启用方向分类，与cls同时使用              | false         |
| rec                | 启用rec文本识别                          | true          |
| enable_mkldnn      | 启用CPU推理加速                          | true          |
| cpu_threads        | CPU线程数                                | 当前 CPU 核数 |
| config_path        | 指定配置文件路径                         | ""            |

- 配置文件用于在启动时注入配置参数，可将参数（如模型库路径等）写在其中，让程序读取它来传入配置。[格式详见此](/docs/详细使用指南.md#4-注入配置参数)。默认情况下，程序启动时读取同目录下 `程序名_config.txt` 的文件。你可传入 `-config_path="路径/配置文件.txt"` 来指定读取哪一个配置文件。（路径需全英文，支持相对路径）
  - 在实际使用中，建议使用手动指定配置文件的方式，这样比较灵活，能一次性指定一组配置参数，而不需要传入一大堆启动参数。
  - 比如：预先写好不同语言模型库的配置文件 `Chinese.txt` 和 `France.txt` ，其中要包含 `det_model_dir` `cls_model_dir` `rec_model_dir` `rec_char_dict_path` 等参数。启动引擎时，想要识别哪种语言，就传入哪个配置文件的路径即可。
- 若目标图片的文字方向不是正朝上，开启`cls`和`use_angle_cls`。否则，保持关闭，减少识别耗时。

#### 动态参数

可以在启动时注入，也可以热更新。

| 键名称         | 值说明                                                                                                                                                           | 默认值      |
| -------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------- |
| limit_side_len | 压缩/放大阈限，单位像素                                                                                                                                          | 960         |
| limit_type     | 为"max"时，若图片长边大于limit_side_len，会压缩至该值，减少识别耗时。为"min"时，若图片短边小于limit_side_len，会放大至该值，增加小尺寸图片的识别率(一般用不到)。 | "max"       |
| visualize      | 启用结果可视化                                                                                                                                                   | false       |
| output         | 启用结果可视化的保存路径，不可中文                                                                                                                               | "./output/" |

- 热更新的意义在于动态更新`limit_side_len`来适应不同大小的图片，避免重启引擎的时间开销。
- 启用结果可视化后，引擎会在每一张识别的图片上绘制文本包围盒，按原来的文件名保存到output目录下。output目录不能为中文。启用可视化时，原文件名不建议为中文，可能会乱码或无法保存。

此外，更多参数详见 [args.cpp](project_files/cpp_infer/src/args.cpp) 。不支持GPU相关、表格识别相关的参数。

### [详细使用指南](docs/详细使用指南.md)

👆当你需要修改或开发新API时欢迎参考。


### 项目构建指南

方式1. [传统构建](https://github.com/hiroi-sora/PaddleOCR-json/blob/main_old_builds/docs/%E9%A1%B9%E7%9B%AE%E6%9E%84%E5%BB%BA%E6%8C%87%E5%8D%97.md)

方式2(推荐). [使用 CMake 快速构建](docs/项目构建指南.md)

### 感谢

本项目中使用了 [nlohmann/json](https://github.com/nlohmann/json) ：
> “JSON for Modern C++”

感谢 [PaddlePaddle/PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR) ，没有它就没有本项目：
> “Awesome multilingual OCR toolkits based on PaddlePaddle”

感谢各位为本项目开发API及贡献代码的朋友！

## 更新日志

版本号链接可前往对应备份分支。

#### [v1.2.1](https://github.com/hiroi-sora/PaddleOCR-json/tree/release/1.2.1) `2022.9.28` 
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
