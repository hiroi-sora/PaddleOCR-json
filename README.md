# PaddleOCR-json 图片转文字程序

图片批量离线OCR文字识别程序。输入图片路径，输出识别结果json字符串，方便别的程序调用。

**本程序的GUI形式：[Umi-OCR 批量图片转文字工具](https://github.com/hiroi-sora/Umi-OCR)**


## 准备工作

下载 [PaddleOCR-json v1.1.1](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/v1.1.1) 并解压，即可。

## 命令行试用

直接打开`PaddleOcr_json.exe`。输入图片路径，回车。
- 任务成功时，输出图片识别信息json字符串。格式见下。
- 任务失败时，输出错误信息json字符串。
- 输出一条信息后，可继续接收路径输入；程序是死循环。
- 支持输入带空格的路径。
- 命令行模式下仅支持英文路径。

程序接受`gbk`编码输入，而输出是`utf-8`。因此，命令行模式下，无法识别输入的非英文字符。

示例：

![](https://tupian.li/images/2022/04/02/image491a4e9ba75069dc.png)

## python调用

- 通过管道与识别器程序交互。
- 支持中文路径：将含中文字符串编码为`gbk`输入管道，即可被正确识别。
- 输入内容必须以换行符结尾。

#### 简单例子

```python
import subprocess
import json

imgPath = "测试文件夹\\测试图片.png\n" # 待检测图片路径，支持中文和空格，结尾必须有换行符。

# 打开管道，启动识别器程序
ret = subprocess.Popen(  
    exePath,
    stdout=subprocess.PIPE,
    stdin=subprocess.PIPE,
)
ret.stdout.readline()  # 读掉第一行

# 发送图片路径，获取识别结果
ret.stdin.write(imgPath.encode("gbk")) # 编码gbk
ret.stdin.flush() # 发送
getStr = self.ret.stdout.readline().decode('utf-8', errors='ignore') # 获取结果，解码utf-8
getObj = json.loads(getStr) # 反序列化json
print("识别结果为：",getObj)
ret.kill()
```


#### 实用例子

[PaddleOCR-demo.py](PaddleOCR-demo.py)

这里代码若与下载的`PaddleOCR-json.rar`中的同名文件不一样，以这里的最新版为准。

## PowerShell调用

[PaddleOCR-demo-powershell.ps1](PaddleOCR-demo-powershell.ps1)

<details>
<summary>这个调用demo不完善，当前存在一些问题</summary>

1. 处理OCR结果的业务函数 OcrFlow 不运行在主窗口中，所以用Windows PowerShell运行代码似乎不会显示它输出的内容，只有在vscode中运行才可以看到。
2. 编码转换问题，得到的 $ocrStr 的内容是utf-8但编码是gbk，因此表现为中文乱码，要重新将字符串编码为utf-8才行。
3. 任务结束退出程序时似乎存在问题，OCR识别器进程能被正常关闭，而powershell进程未结束，exit不起作用。（或许需要强制杀死本进程$pid？）

</details>

## 输出值JSON说明

`PaddleOcr_json.exe` 将把识别信息以json格式字符串的形式打印到控制台。根含且只含两个元素：状态码`code`和内容`data`。状态码code为整数，每种状态码对应一种data形式：

##### 识别到文字（100）

- data内容为数组。数组每一项为字典，含三个元素：
  - `text` ：文本内容，字符串。
  - `box` ：文本包围盒，长度为8的数组，分别为左上角xy、右上角xy、右下角xy、左下角xy。整数。
  - `score` ：识别置信度，浮点数。

##### 未识别到文字（101）

- data为字符串：`No text found in image.`

##### 加载图片错误（200）

- data为字符串：`Failed to load image form file "{图片路径}".`


## 载入多国语言语言 & 切换模型库

请参考 [Umi-OCR#识别器设置](https://github.com/hiroi-sora/Umi-OCR#%E8%AF%86%E5%88%AB%E5%99%A8%E8%AE%BE%E7%BD%AE)


## 开发说明

### 部署项目

1. 下载并编译飞桨PaddleOCR C++预测库，可参考 [这篇教程](https://cloud.tencent.com/developer/article/1843504) 。
2. 将本项目`src`、`include`目录中的文件替换进去。

### 注意避坑

- 为何通过控制台传入图片路径而不是通过启动参数传递？因为本程序初始化时要载入不少模型库文件，若每次启动程序只识别一张图片，则批量处理会多出额外的初始化开销。所以采用了一次初始化，后续多次传入路径的设计。
- 理论上，程序对于传入的异常文件会有处理，返回错误码200；不影响后续工作。（异常文件指：不存在的路径，opencv不支持的图片格式，已损坏的图片。）
    但使用中，若通过管道传入的gbk编码**含中文路径**若存在异常（如`测试文件夹\\已损坏的图片.png`），大多数情况下能处理，有小概率**进程直接挂掉**。如果是纯英文路径（如`test\\damaged image.png`），则貌似没有这个问题。目前尚未清楚原因，目测跟opencv的imread()有关。
    当然，含中文路径不存在异常时，不会出现以上bug。
    建议，如果已知要处理的图片 有出现异常的风险，则调用OCR前先判断一下。
- 我使用VS2019编写这个项目。代码中所有以**中文结尾**的**注释**，必须多加一个**空格**；否则它会吞掉行尾换行符导致编译器把下一行也当成注释。编辑器并不会提示这个错误，编译的时候才会不通过。

### 感谢

本项目中使用了 [configor](https://github.com/Nomango/configor) ：
> “一个为 C++11 量身打造的轻量级 config 库，轻松完成 JSON 解析和序列化功能，并和 C++ 输入输出流交互。”


## 更新日志

#### v1.1.1 `2022.4.16` 
- 修正了漏洞：当`文本检测`识别到区域但`文本识别`未在区域中检测到文字时，可能输出不相符的包围盒。

#### v1.1.0 `2022.4.2` 
- 修改了json输出格式，改为状态码+内容，便于调用方判断。

#### v1.0 `2022.3.28`
