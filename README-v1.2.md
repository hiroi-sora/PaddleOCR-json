# PaddleOCR-json 图片转文字程序 

图片批量离线OCR文字识别程序。传入图片路径，输出识别结果json字符串，方便别的程序调用。

<font color='red'> v1.2.0 版本还在开发中，已发现存在BUG。</font>建议使用功能稳定的 [v1.1.1版本](backups_previous_version/PaddleOCR-json_v1.1.1) 。

~~`v1.2.0` 重构了整个项目，增加了一些新功能和潜在的新BUG。若遇到问题请提issue，或者使用功能稳定的 [v1.1.1版本](backups_previous_version/PaddleOCR-json_v1.1.1) 。~~

**本程序的GUI形式：[Umi-OCR 批量图片转文字工具](https://github.com/hiroi-sora/Umi-OCR)**


## 准备工作

~~下载 [PaddleOCR-json v1.2.0](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/v1.2.0) 并解压，即可。~~

## 命令行试用

直接打开`PaddleOCR_json.exe`。控制台会输出一串日志信息，最后清屏，输出：
```
Active code page: 65001
OCR initialization completed.
```
即表示各项参数和模型库加载完成，OCR初始化完毕。随后可传入图片路径来识别。

传入路径方式：直接在控制台输入图片的绝对或相对路径，回车。一段时间后显示识别json内容。

- 输出一条信息后，可继续接收路径输入；程序此时为死循环。
- 支持输入带空格的路径。
- 命令行模式下仅支持英文路径。

程序接受`gbk`编码输入，而输出是`utf-8`。因此，命令行模式下，无法识别输入的非英文（ascii）字符。

示例：

![img-1.jpg](https://tupian.li/images/2022/08/26/img-1.jpg)

## 详细使用说明

### 传入配置信息

配置信息规定OCR的各项属性和识别模型库的路径，可通过多种方式传入程序。

1. 默认参数：程序内自带一套适用于PaddleOCR v3版本模型库的参数，但是不包含模型库路径。即，用户至少要传入模型库和字典路径，才能启动程序。另，若使用v2版模型库，则必须传入参数`rec_img_h=32`。
2. 默认配置文件：程序启动时读取同目录下 `程序名_config.txt`。若该文件存在，则读取其中的配置信息。（例：程序名为`PaddleOCR_json.exe`，则默认配置参数为`PaddleOCR_json_config.txt`）
3. 指定配置文件：可在启动参数中传入 `--config_path=配置文件.txt` ，指定另一个配置文件。此时忽略默认配置文件。支持绝对/相对/带空格路径（加引号）。
4. 启动参数：可通过启动参数传入配置信息。

以上4种参数的优先级，越往下越高。即，存在同一个配置项冲突时，优先采用 启动参数 > 配置文件 > 默认 。

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

### 配置信息说明

比较重要的配置项的说明如下

| 键名称             | 值说明                | 默认值 |
| ------------------ | --------------------- | ------ |
| det_model_dir      | det库路径             | 必填   |
| cls_model_dir      | cls库路径             | 必填   |
| rec_model_dir      | rec库路径             | 必填   |
| rec_char_dict_path | rec字典路径           | 必填   |
| rec_img_h          | v3模型填48，v2填32    | 48     |
| ensure_ascii       | 填true启用ascii转义   | false  |
| limit_side_len     | 压缩阈限              | 960    |
| use_system_pause   | 填false时停用自动暂停 | true   |

- 当调用方难以处理管道utf-8转码时，设`--ensure_ascii=true`，本程序将输出以ascii字符编码utf-8信息，以此规避乱码问题。例：`测试字符串`→`\u6d4b\u8bd5\u5b57\u7b26\u4e32`。
- 当传入超大分辨率图片(4K)、且图片中含有小字时，调高`limit_side_len`的值，减少压缩以提高精准度。可调至与图片高度一致。但这将大幅增加识别耗时。
- 默认退出程序前（如单次识别完毕，或有报错），程序会通过 `system("pause")` 自动暂停以便检查。若调用时不想暂停，则可设 `--use_system_pause=false`。

<details>
<summary>所有配置项定义如下</summary>

```
// 数据类型 (键名称, 默认值, 说明)
// common args
DEFINE_bool(use_gpu, false, "Infering with GPU or CPU.");
DEFINE_bool(use_tensorrt, false, "Whether use tensorrt.");
DEFINE_int32(gpu_id, 0, "Device id of GPU to execute.");
DEFINE_int32(gpu_mem, 4000, "GPU id when infering with GPU.");
DEFINE_int32(cpu_threads, 10, "Num of threads with CPU.");
DEFINE_bool(enable_mkldnn, false, "Whether use mkldnn with CPU.");
DEFINE_string(precision, "fp32", "Precision be one of fp32/fp16/int8");
DEFINE_bool(benchmark, false, "Whether use benchmark.");
DEFINE_string(output, "./output/", "Save benchmark log path.");
DEFINE_string(image_dir, "", "Dir of input image.");
DEFINE_string(
    type, "ocr",
    "Perform ocr or structure, the value is selected in ['ocr','structure'].");
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

// structure model related
DEFINE_string(table_model_dir, "", "Path of table struture inference model.");
DEFINE_int32(table_max_len, 488, "max len size of input image.");
DEFINE_int32(table_batch_num, 1, "table_batch_num.");
DEFINE_string(table_char_dict_path, "", "Path of dictionary.");

// ocr forward related
DEFINE_bool(det, true, "Whether use det in forward.");
DEFINE_bool(rec, true, "Whether use rec in forward.");
DEFINE_bool(cls, false, "Whether use cls in forward.");
DEFINE_bool(table, false, "Whether use table structure in forward.");

// 读取配置文件 
DEFINE_string(config_path,"","Path of config txt.");

// 输出json字符串转ascii编码 
DEFINE_bool(ensure_ascii, false, "Whether characters in the output are escaped with sequences to ASCII.");
```

</details>

### 过滤启动日志

程序启动时会输出大量日志信息，初始化完成后输出`OCR initialization completed.`。当你用别的程序调用本程序时，建议先循环读取过滤掉启动日志，直到读到完成标志，再进入正式工作。以python为例：

```python
while "OCR initialization completed." not in str(ret.stdout.readline()):
    pass
```

### 传入图片路径

程序有两种方式处理图片：

1. 当通过启动参数/配置文件设定了 `image_dir=图片路径` 的值时，识别该图片，输出结果，然后直接结束程序。
2. 当未设定 `image_dir` 时，程序进入循环模式.初始化完成后，可以通过控制台或管道输入一张图片路径，该图片识别完毕输出结果后，可以继续输入并识别下一张图片。

### 输出值JSON说明

`PaddleOcr_json.exe` 将把识别信息以json格式字符串的形式打印到控制台。根含且只含两个元素：状态码`code`和内容`data`。状态码code为整数，每种状态码对应一种data形式：

##### 识别到文字（100）

- data内容为数组。数组每一项为字典，含三个元素：
  - `text` ：文本内容，字符串。
  - `box` ：文本包围盒，长度为8的数组，分别为左上角xy、右上角xy、右下角xy、左下角xy。整数。
  - `score` ：识别置信度，浮点数。
- 例：`{"code":100,"data":[{"box":[24,27,234,27,234,53,24,53],"score":0.9904433488845825,"text":"飞舞的因果交流"}]}`

##### 未识别到文字（101）

- data为字符串：`No text found in image. Path:"{图片路径}"`

##### 图片路径不存在（200）

- data为字符串：`Image path not exist. Path:"{图片路径}".`
- 此时请检查图片路径是否正确。含空格的路径，通过启动参数传入时需要加引号，通过控制台/管道直接传入时无需引号。含中文的路径，只能通过管道编码为`gbk`后输入。

##### 无法读取图片（201）

- data为字符串：`Image read failed. Path"{图片路径}".`
- 此时请检查图片格式是否符合opencv支持；图片本身是否已损坏。



## python调用


#### 简单示例1（通过管道传路径）

- 通过管道与识别器程序交互。
- 支持中文路径：将含中文字符串编码为`gbk`输入管道，即可被正确识别。
- 输入内容必须以换行符结尾。

```python
import subprocess
import json
import os

imgPath = "E:\\test2.jpg\n"  # 待检测图片路径，支持中文和空格，结尾必须有换行符。
exePath = r"E:\MyCode\CppCode\PaddleOCR\cpp_infer\build\Release\PaddleOCR_json.exe"

# 打开管道，启动识别器程序
ret = subprocess.Popen(
    exePath,
    cwd=os.path.abspath(os.path.join(exePath, os.pardir)),
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
)
# 过滤初始化语句
while(1):
    r = str(ret.stdout.readline())
    print(r)
    if "OCR initialization completed." in r:
        break

# 发送图片路径，获取识别结果
ret.stdin.write(imgPath.encode("gbk"))  # 编码gbk
ret.stdin.flush()  # 发送
getStr = ret.stdout.readline().decode(
    'utf-8', errors='ignore')  # 获取结果，解码utf-8
getObj = json.loads(getStr)  # 反序列化json
print("识别结果为：", getObj)
ret.kill()  # 结束进程
```

#### 简单示例2（通过启动参数传路径）

- 通过管道接收识别器的返回值。
- 支持中文路径：含中文字符串无需编码，直接拼接入启动参数。若含空格，需要以双引号包裹。

```python
import subprocess
import json
import os

imgPath = "E:\\test2.jpg"  # 待检测图片路径，支持中文和空格，结尾不能有换行符。
exePath = r"E:\MyCode\CppCode\PaddleOCR\cpp_infer\build\Release\PaddleOCR_json.exe"
# 拼接启动参数。设use_system_pause以让进程结束后不暂停，自动退出。
beginStr = f'{exePath} --use_system_pause=0 --image_dir="{imgPath}"'

# 打开管道，启动识别器程序
ret = subprocess.Popen(
    beginStr,
    cwd=os.path.abspath(os.path.join(exePath, os.pardir)),
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
)
# 过滤初始化语句，简便写法
while "OCR initialization completed." not in str(ret.stdout.readline()):
    pass

# 获取识别结果
getStr = ret.stdout.readline().decode(
    'utf-8', errors='ignore')  # 获取结果，解码utf-8
getObj = json.loads(getStr)  # 反序列化json
print("识别结果为：", getObj)
# 无需kill，进程自动结束。
```

### 载入多国语言语言&切换模型库

新版本README还未写好，可先参考[旧版](backups_previous_version/PaddleOCR-json_v1.1.1/README.md#载入多国语言语言)。

### [项目构建指南](project_files/README.md)

👆当你需要修改本项目代码时欢迎参考。

### 感谢

本项目中使用了 [nlohmann/json](https://github.com/nlohmann/json) ：
> “JSON for Modern C++”


## 更新日志

#### v1.2.0 Beta `2022.8.26` 
- 重构整个工程，核心代码同步PaddleOCR 2.6。
- 对v3版识别库的支持更好。
- 新功能：启动参数。
- 新功能：ascii转义。（感谢 @AutumnSun1996 的提议[issue #4](https://github.com/hiroi-sora/PaddleOCR-json/issues/4)）
- 已发现bug，修改中。

#### v1.1.1 `2022.4.16` 
- 修正了漏洞：当`文本检测`识别到区域但`文本识别`未在区域中检测到文字时，可能输出不相符的包围盒。

#### v1.1.0 `2022.4.2` 
- 修改了json输出格式，改为状态码+内容，便于调用方判断。

#### v1.0 `2022.3.28`
