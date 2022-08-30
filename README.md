# PaddleOCR-json 图片转文字程序 v1.2.0

Windows端图片批量离线OCR文字识别程序。通过管道等多种方式输入图片路径，输出识别结果json字符串，方便别的程序调用。

示例：

![img-1.jpg](https://tupian.li/images/2022/08/26/img-1.jpg)

`v1.2.0` 重构了整个项目，同步 [PaddleOCR 2.6 (2022.8.24) cpu_avx_mkl](https://github.com/PaddlePaddle/PaddleOCR/tree/release/2.6) ，提升性能、增加了亿些新功能和潜在的新BUG。遇到问题请提issue，或者使用功能稳定的 [v1.1.1版本](backups_previous_version/PaddleOCR-json_v1.1.1) 。

- **方便** ：解压即用，无需安装和配置环境，无需联网。引入api，两行代码调用OCR。（未提供api的语言，可参考本文档通过管道调用OCR）
- **高速** ：`v1.2.0` 比前代提速20%。默认启用mkldnn加速，字少的图片耗时在400ms以内，比在线OCR服务更快。
- **精准** ：`v1.2.0` 附带PPOCR-v3识别库，比前代对非常规字形（手写、艺术字、小字、杂乱背景等）具有更佳的识别率。

**本程序的GUI形式：[Umi-OCR 批量图片转文字工具](https://github.com/hiroi-sora/Umi-OCR)**


## 准备工作

下载 [PaddleOCR-json v1.2.0](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/v1.2.0) 并解压，即可。


## 通过API调用

通过api调用时，已处理好gbk/utf-8转码和json反序列化等工作。调用方只管传入图片路径、接收返回字典。

### 1. [Python API](api/python)

将 [PPOCR_api.py](api/python/PPOCR_api.py) 引入你的项目。

<details>
<summary>使用示例：</summary>

```python
from PPOCR_api import PPOCR

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
ocr = PPOCR('D:\…………\PaddleOCR-json\PaddleOCR_json.exe')

# 识别图片，传入图片路径
while True:
    imgPath = input('请输入图片路径，退出直接回车：')
    if imgPath:
        getObj = ocr.run(imgPath)
        print(f'图片识别完毕，状态码：{getObj["code"]} 结果：\n{getObj["data"]}\n')
    else:
        break

del ocr  # 销毁识别器对象，结束子进程。
print('程序结束。')
```

</details>

### 2. [PowerShell API](api/powershell)

将 [PPOCR_api.ps1](api/powershell/PPOCR_api.ps1) 引入你的项目。

<details>
<summary>使用示例：</summary>

```PowerShell
Import-Module -Force D:\…………\PPOCR_api.ps1

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
$ocr = [PPOCR]::new("D:\…………\PaddleOCR-json\PaddleOCR_json.exe")

# 识别图片，传入图片路径
while (1) {
    $imgPath = read-host "请输入图片路径，退出直接回车" 
    if ($imgPath) {
        $getObj = $ocr.run($imgPath)
        Write-Host "图片识别完毕，状态码：$($getObj.'code') 结果：`n"($getObj.'data')"`n"
    }
    else {
        break
    }
}

$ocr.del()  # 结束子进程。
Write-Host "程序结束。"
```

</details>

### 更多语言API

待补充……

## 输出JSON说明

`PaddleOcr_json.exe` 将把图片转文字识别信息以json格式字符串的形式打印到控制台。根含两个元素：状态码`code`和内容`data`。在设置了热更新的回合，还会含有额外元素：参数更新日志`hotUpdate`。

状态码`code`为整数，每种状态码对应一种情况：

##### 识别到文字（100）

- data内容为数组。数组每一项为字典，含三个元素：
  - `text` ：文本内容，字符串。
  - `box` ：文本包围盒，长度为8的数组，分别为左上角xy、右上角xy、右下角xy、左下角xy。整数。
  - `score` ：识别置信度，浮点数。
- 例：
  ```
    {"code":100,"data":[{"box":[24,27,234,27,234,53,24,53],"score":0.9904433488845825,"text":"飞舞的因果交流"}]}
    ```

##### 未识别到文字（101）

- data为字符串：`No text found in image. Path:"{图片路径}"`
- 这是正常现象，识别没有文字的空白图片时会出现这种结果。
- 例：
    ```
    {"code":101,"data":"No text found in image. Path:\"D:\\blank.png\""}
    ```

##### 图片路径不存在（200）

- data为字符串：`Image path not exist. Path:"{图片路径}".`
- 此时请检查图片路径是否正确。通过控制台或管道传入中文路径时，务必编码为`gbk`或使用ascii转义的json字符串。程序无法识别直接输入的utf-8字符。

##### 无法读取图片（201）

- data为字符串：`Image read failed. Path:"{图片路径}".`
- 此时请检查图片格式是否符合opencv支持；或图片本身是否已损坏。

##### JSON格式化失败（300）

- data为字符串：`JSON dump failed. Coding error.`
- 此问题基本上是由于管道输入了utf-8字符引起。

##### hotUpdate元素

- `hotUpdate` 元素仅在设置了热更新的回合出现，与识别码`code`没有关联。其内容为记录热更新日志的字符串。
- 例：
    ```
    {"code":200,"data":"Image path not exist. Path:\"\"","hotUpdate":"det_model_dir set to ch_PP-OCRv2_det_infer. limit_side_len set to 961. rec_img_h set to 32. "}
    ```

## 命令行调用方式简介

简单介绍一下调用程序识别图片的三种方式。

#### 方式1：启动参数

1. 打开命令提示符cmd，跳转到本程序目录下。
    ```
    cd /d D:\……\PaddleOCR-json
    ```
2. 启动 `PaddleOCR_json` ，并通过`-image_dir`参数传入一张图片的路径。
   - 支持中文和空格路径。建议加双引号将路径字符串裹起来。
    ```
    PaddleOCR_json -image_dir="测试图片/test 1.jpg"
    ```
3. 程序初始化，输出一大串日志信息；然后识别该图片，打印json字符串。随后自动结束任务并退出程序。

#### 方式2：直接输入路径

1. 双击打开 `PaddleOCR_json.exe` 。控制台会打印一大串日志信息，最后显示以下两行，表示OCR初始化完毕，可进行批量识图操作。
    ```
    Active code page: 65001
    OCR init completed.
    ```
2. 直接在控制台输入图片路径，回车。一段时间后显示识别json内容。
    ```
    D:/test images/test 1.jpg
    ```
3. 识别完后程序不会退出，可以继续输入路径识别新的图片。
   - 支持带空格的路径，无需引号。
   - 此时仅支持手动输入纯英文(ascii字符)的路径。不过，调用方将含中文路径编码为gbk字符串再输入管道也可以让本程序识别。

#### 方式3：直接输入json（经过ascii转义）

1. 同方式2，打开 `PaddleOCR_json.exe` ，等待初始化完成。
2. 在控制台输入json字符串 `{"image_dir":"图片路径"}`。
   - 当路径含中文(非ascii字符)时，必须经过ascii转义，即将文字转换成`\uxxxx`的字符串。这样可以规避gbk编码问题。
    ```
    {"image_dir": "\u6D4B\u8BD5\u56FE\u7247\\test 1.jpg"}
    ```
3. 一段时间后显示识别json内容。同方式2，可以继续识别下一张。

## 详细使用说明

用别的程序调用本程序，核心问题是进程间通信。

### 1. 启动程序，重定向管道

- Window下有三个标准io管道：标准输入stdin，标准输出stdout，标准错误输出stderr。调用方至少要重定向本程序stdin和stdout。此外，还要指定工作目录cwd（即PaddleOCR_json.exe所在的目录）。以python为例：
    ```python
    ret = subprocess.Popen(
        "程序目录\\PaddleOCR_json.exe", cwd="程序目录",
        stdout=subprocess.PIPE, stdin=subprocess.PIPE,  )
    ```

### 2. 监测启动完成

- 本程序启动时，第三方链接库会打印大量日志信息。不过绝大多数日志输出在 `stderr` 流，可以不去管它。
- 程序在 `stdout` 输出 `OCR init completed.` 标志着初始化完成。调用方应该先循环读取直到读到完成标志，再进入正式工作。以python为例：

    ```python
    while "OCR init completed." not in str(ret.stdout.readline()):
        if not self.ret.poll() == None:  # 子进程不在运行
            raise Exception(f'子进程已结束，OCR初始化失败')
    ```

### 3. 传入图片路径 & 获取输出信息

- 程序有四种方式输入图片路径，分别是 ①管道直接输入路径；②管道输入json；③启动参数；④写在配置文件中。

- 其中③和④并不实用，关注①和②即可。因为③和④是一次性的手段，程序识别完第一张图片后自动退出。每次启动程序会消耗初始化和暖机的时间。批量识别多张图片时，重复启动，将造成极大的浪费开销。

<details>
<summary>①管道输入路径</summary>

##### 通过管道传路径

- 支持中文路径：将含中文字符串编码为`gbk`输入管道，即可被正确识别。
- 输入管道的字符串必须以换行符结尾，一次只能输入一条图片路径。
- 以python为例：

```python
import subprocess
import json
import os

imgPath = "E:\\test2.jpg\n"  # 待检测图片路径，支持中文和空格，结尾必须有换行符。
exePath = r"E:\…………\PaddleOCR_json.exe"

# 打开管道，启动识别器程序
ret = subprocess.Popen(
    exePath,
    cwd=os.path.abspath(os.path.join(exePath, os.pardir)),
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
)
# 过滤初始化语句
while "OCR init completed." not in str(ret.stdout.readline()):
    if not self.ret.poll() == None:  # 子进程不在运行
        raise Exception(f'子进程已结束，OCR初始化失败')

# ↓↓ 发送图片路径，获取识别结果
ret.stdin.write(imgPath.encode("gbk"))  # 编码gbk
ret.stdin.flush()  # 发送
getStr = ret.stdout.readline().decode(
    'utf-8', errors='ignore')  # 获取结果，解码utf-8
getObj = json.loads(getStr)  # 反序列化json
print("识别结果为：", getObj)
# ↑↑ 可重复进行，批量识别图片。
ret.kill()  # 调用方结束识别器进程
```

</details>

<details>
<summary>②管道输入json</summary>

##### 通过管道传json

- 支持中文路径：将含中文字符串装填入json、设定ascii转义，输入管道，即可被正确识别。
- 输入管道的json字符串必须以换行符结尾，一次只能输入一条json。
- json中还可以包含其它参数进行热更新，详见后续。
- 以python为例：

```python
import subprocess
import json
import os

imgPath = "D:\图片\Screenshots\测试图片\幸运草 (2).png"  # 待检测图片路径，支持中文和空格，结尾无换行符。
exePath = r"C:\MyCode\CppCode\PaddleOCR\cpp_infer\build\Release\PaddleOCR_json.exe"

# 打开管道，启动识别器程序
ret = subprocess.Popen(
    exePath,
    cwd=os.path.abspath(os.path.join(exePath, os.pardir)),
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
)
# 过滤初始化语句
while "OCR init completed." not in str(ret.stdout.readline()):
    if not self.ret.poll() == None:  # 子进程不在运行
        raise Exception(f'子进程已结束，OCR初始化失败')
# ↓↓ 发送图片路径，获取识别结果
imgObj = {"image_dir": imgPath}
#                    开启ascii转义，关闭缩进。最后加个换行符。
imgStr = json.dumps(imgObj, ensure_ascii=True, indent=None)+"\n"
ret.stdin.write(imgStr.encode())  # 无需指定哪种编码，因为ascii字符都一样
ret.stdin.flush()  # 发送
getStr = ret.stdout.readline().decode(
    'utf-8', errors='ignore')  # 获取结果，解码utf-8
getObj = json.loads(getStr)  # 反序列化json
print("识别结果为：", getObj)
# ↑↑ 可重复进行，批量识别图片。
ret.kill()  # 调用方结束识别器进程
```

</details>


### 4. 注入配置参数

配置信息规定OCR的各项属性和识别模型库的路径，可通过多种方式注入程序。正常情况下，使用默认值即可。若您对PPOCR有足够了解并想修改参数以适应自己的任务需求，请参考本节。

<details>
<summary>展开详情</summary>

1. 默认参数：程序内自带一套适用于PaddleOCR v3版本模型库的参数，但是不包含模型库路径。即，用户至少要传入模型库和字典路径，才能启动程序。另，若使用v2版模型库，则必须传入参数`rec_img_h=32`。
2. 默认配置文件：程序启动时读取同目录下 `程序名_config.txt`。若该文件存在，则读取其中的配置信息。（例：程序名为`PaddleOCR_json.exe`，则默认配置参数为`PaddleOCR_json_config.txt`）
3. 指定配置文件：可在启动参数中传入 `--config_path=配置文件.txt` ，指定另一个配置文件。此时忽略默认配置文件。支持绝对/相对/带空格路径（加引号）。
4. 启动参数：可通过启动参数传入配置信息。
5. 热更新：程序启动后，通过json传入任意配置项。无需重新初始化即可修改参数。但可能存在未知的隐患，建议谨慎使用并多做测试。
   - 注意，各识别库和字典**路径**等参数不能热更新（它们只在初始化时生效）。
   - 热更新和识别图片可以在同一回合进行，即json里同时填了`image_dir`和别的参数。
   - 热更新的回合，返回值json里会额外多出`hotUpdate`元素，值为热更新日志字符串。

以上5种参数的优先级，越往下越高。即，存在同一个配置项冲突时，优先采用 热更新 > 启动参数 > 配置文件 > 默认 。

- 热更新的格式是：`{"键":值}`
  - 例：`{"use_angle_cls":false, "rec_img_h":32}`
- 启动参数的格式是：`--键=值 `
  - 例：`PaddleOCR_json  --use_angle_cls=false --rec_img_h=32 --image_dir="E:\测试\img 1.jpg"`
- 配置文件的格式是：`键 值`，支持#开头的注释（只能在最前面写#来注释一整行）。例：
    ```
    # det 检测模型库
    det_model_dir ch_PP-OCRv2_det_infer
    # cls 方向分类器库
    cls_model_dir ch_ppocr_mobile_v2.0_cls_infer
    # rec 识别模型库
    rec_model_dir ch_PP-OCRv2_rec_infer
    # 字典路径
    rec_char_dict_path ppocr_keys_v1.txt
    ```

</details>

### 5. 配置信息说明

<details>
<summary>重要配置项说明如下</summary>

| 键名称             | 值说明                   | 默认值 |
| ------------------ | ------------------------ | ------ |
| det_model_dir      | det库路径                | 必填   |
| cls_model_dir      | cls库路径                | 必填   |
| rec_model_dir      | rec库路径                | 必填   |
| rec_char_dict_path | rec字典路径              | 必填   |
| rec_img_h          | v3模型填48，v2填32       | 48     |
| ensure_ascii       | 填true启用ascii转义      | false  |
| limit_side_len     | 压缩阈限                 | 960    |
| use_system_pause   | 填false时停用自动暂停    | true   |
| enable_mkldnn      | 填false时停用CPU推理加速 | true   |

- 当调用方难以处理管道utf-8转码时，设`--ensure_ascii=true`，本程序将输出以ascii字符转义的utf-8文本（\uxxxx格式），以此规避乱码问题。例：`测试字符串 → \u6d4b\u8bd5\u5b57\u7b26\u4e32`。这个配置项只影响输出，不影响输入图片路径时的编码。
- 当传入超大分辨率图片(4K)、且图片中含有小字时，调高`limit_side_len`的值，减少压缩以提高精准度。可调至与图片高度一致。但这将大幅增加识别耗时。
- 默认退出程序前（如单次识别完毕，或有报错），程序会通过 `system("pause")` 自动暂停以便检查。若调用时不想暂停，则可设 `--use_system_pause=false`。
- 十分建议启用CPU推理加速 `--enable_mkldnn=true` ，这会大幅提高识别速度。虽然是Intel家的技术，但实测它在AMD锐龙上运行良好。若您的硬件有兼容性问题，尝试关闭这个选项。

</details>

<details>
<summary>所有配置项定义如下</summary>

```
// 数据类型 (键名称, 默认值, 说明)
// common args
DEFINE_bool(use_gpu, false, "Infering with GPU or CPU."); // 无效，GPU不可用
DEFINE_bool(use_tensorrt, false, "Whether use tensorrt."); // 无效，GPU不可用
DEFINE_int32(gpu_id, 0, "Device id of GPU to execute."); // 无效，GPU不可用
DEFINE_int32(gpu_mem, 4000, "GPU id when infering with GPU."); // 无效，GPU不可用
DEFINE_int32(cpu_threads, 10, "Num of threads with CPU.");
DEFINE_bool(enable_mkldnn, true, "Whether use mkldnn with CPU.");
DEFINE_string(precision, "fp32", "Precision be one of fp32/fp16/int8");
DEFINE_bool(benchmark, false, "Whether use benchmark.");
DEFINE_string(output, "./output/", "Save benchmark log path.");
DEFINE_string(image_dir, "", "Dir of input image.");
DEFINE_string(type, "ocr","Perform ocr or structure, the value is selected in ['ocr','structure']."); // 无效，表格识别不可用
// detection related
DEFINE_string(det_model_dir, "", "Path of det inference model.");
DEFINE_string(limit_type, "max", "limit_type of input image.");
DEFINE_int32(limit_side_len, 960, "limit_side_len of input image.");
DEFINE_double(det_db_thresh, 0.3, "Threshold of det_db_thresh.");
DEFINE_double(det_db_box_thresh, 0.6, "Threshold of det_db_box_thresh.");
DEFINE_double(det_db_unclip_ratio, 1.5, "Threshold of det_db_unclip_ratio.");
DEFINE_bool(use_dilation, false, "Whether use the dilation on output map.");
DEFINE_string(det_db_score_mode, "slow", "Whether use polygon score.");
DEFINE_bool(visualize, false, "Whether show the detection results.");
// classification related
DEFINE_bool(use_angle_cls, false, "Whether use use_angle_cls.");
DEFINE_string(cls_model_dir, "", "Path of cls inference model.");
DEFINE_double(cls_thresh, 0.9, "Threshold of cls_thresh.");
DEFINE_int32(cls_batch_num, 1, "cls_batch_num.");
// recognition related
DEFINE_string(rec_model_dir, "", "Path of rec inference model.");
DEFINE_int32(rec_batch_num, 6, "rec_batch_num.");
DEFINE_string(rec_char_dict_path, "", "Path of dictionary.");
DEFINE_int32(rec_img_h, 48, "rec image height");
DEFINE_int32(rec_img_w, 320, "rec image width");

// structure model related 无效，表格识别不可用
DEFINE_string(table_model_dir, "", "Path of table struture inference model.");
DEFINE_int32(table_max_len, 488, "max len size of input image.");
DEFINE_int32(table_batch_num, 1, "table_batch_num.");
DEFINE_string(table_char_dict_path, "", "Path of dictionary.");

// ocr forward related
DEFINE_bool(det, true, "Whether use det in forward.");
DEFINE_bool(rec, true, "Whether use rec in forward.");
DEFINE_bool(cls, false, "Whether use cls in forward.");
DEFINE_bool(table, false, "Whether use table structure in forward."); // 无效，表格识别不可用

// 配置文件路径 
DEFINE_string(config_path,"","Path of config txt.");

// 输出json字符串转ascii编码 
DEFINE_bool(ensure_ascii, false, "Whether characters in the output are escaped with sequences to ASCII.");

// 退出前暂停程序 
DEFINE_bool(use_system_pause, true, "Whether system(\"pause\") before exit");
```

</details>

### 载入多国语言语言&切换模型库

新版本README还未写好，可先参考[旧版](backups_previous_version/PaddleOCR-json_v1.1.1/README.md#载入多国语言语言)。

### [项目构建指南](project_files/README.md)

👆当你需要修改本项目代码时欢迎参考。

### 感谢

本项目中使用了 [nlohmann/json](https://github.com/nlohmann/json) ：
> “JSON for Modern C++”


## 更新日志

#### v1.2.0 `2022.8.29` 
- 修复了一些BUG，~~新添了亿些BUG~~。
- 增强了面对不合法编码时的健壮性。
- 默认开启mkldnn加速。
- 新功能：json输入及热更新。

#### v1.2.0 Beta `2022.8.26` 
- 重构整个工程，核心代码同步PaddleOCR 2.6。
- 对v3版识别库的支持更好。
- 新功能：启动参数。
- 新功能：ascii转义。（感谢 @AutumnSun1996 的提议 [issue #4](https://github.com/hiroi-sora/PaddleOCR-json/issues/4) ）

#### [v1.1.1]((backups_previous_version/PaddleOCR-json_v1.1.1)) `2022.4.16` 
- 修正了漏洞：当`文本检测`识别到区域但`文本识别`未在区域中检测到文字时，可能输出不相符的包围盒。

#### v1.1.0 `2022.4.2` 
- 修改了json输出格式，改为状态码+内容，便于调用方判断。

#### v1.0 `2022.3.28`
