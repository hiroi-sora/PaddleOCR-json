# PaddleOCR-json Python API

使用这份API，可以方便地调用 PaddleOCR-json 。比起Python原生的PaddleOCR库，PaddleOCR-json拥有更好的性能。你可以同时享受C++推理库的高效率和Python的简易开发。

Python API 拥有三大模块：
- 基础OCR接口
- 结果可视化模块，将OCR结果绘制到图像上并展示或保存。
- 文本后处理模块，支持段落合并、竖排文本整理等功能。

# 基础OCR接口

```python
from PPOCR_api import GetOcrApi
```

### 调用OCR的流程分为三步：
1. 初始化OCR引擎进程
2. 通过OCR引擎，执行一次或多次识图任务
3. 关闭OCR引擎进程

### 第一步：初始化

**接口：** `GetOcrApi()`

**参数：** 

| 名称     | 默认值 | 类型 | 描述                                                           |
| -------- | ------ | ---- | -------------------------------------------------------------- |
| exePath  | 必填   | str  | 引擎exe的路径，如`D:/PaddleOCR-json.exe`                       |
| argument | None   | dict | 启动参数字典。可以用这个参数指定配置文件、指定识别语言。       |
| ipcMode  | "pipe" | str  | 进程间通信方式，可选值为套接字模式`socket` 或 管道模式`pipe`。 |

**返回值：** 

初始化成功，返回引擎API对象。初始化失败，抛出异常。

**示例1：** 最简单的情况

```python
ocr = GetOcrApi(r"…………\PaddleOCR_json.exe")
```

**示例2：** 指定使用繁体中文识别库（需要先在引擎models目录内放入识别库文件）

注意，config_path的路径如果是相对路径，则根为PaddleOCR-json.exe所在的路径，而不是Python脚本的路径。

```python
argument = {'config_path': "models/config_chinese_cht.txt"}
ocr = GetOcrApi(r"…………\PaddleOCR_json.exe", argument)
```

**示例3：** 指定使用套接字通信方式

使用管道通信(默认)和套接字通信，在使用上而言是透明的，即调用方法完全一致。

性能上有微弱的区别，管道的效率略高一点，而套接字TCP在大型数据传输时（如30MB以上的Base64图片数据）可能稳定性略好一些。对于普通用户，使用默认设定即可。

```python
ocr = GetOcrApi(r"…………\PaddleOCR_json.exe", ipcMode="socket")
```

### 第二步：识别图片

Python API 提供了丰富的接口，可以用各种姿势调用OCR。

#### 1. 识别本地图片

**方法：** `run()`

**说明：** 对一张本地图片进行OCR

**参数：** 

| 名称    | 默认值 | 类型 | 描述                                 |
| ------- | ------ | ---- | ------------------------------------ |
| imgPath | 必填   | str  | 识别图片的路径，如`D:/test/test.png` |

**返回值字典：** 

| 键   | 类型 | 描述                                                    |
| ---- | ---- | ------------------------------------------------------- |
| code | int  | 状态码。识别成功且有文字为100。其他情况详见主页README。 |
| data | list | 识别成功时，data为OCR结果列表。                         |
| data | str  | 识别失败时，data为错误信息字符串。                      |

**示例：** 

```python
res = ocr.run("test.png")
print("识别结果：\n", res)
```

#### 2. 识别剪贴板图片

**方法：** `runClipboard()`

**说明：** 对当前剪贴板首位的图片进行OCR

**无参数** 

**返回值字典：同上** 

**示例：** 

```python
res = ocr.runClipboard()
print("剪贴板识别结果：\n", res)
```

#### 3. 识别图片字节流

**方法：** `runBytes()`

**说明：** 对一个图片字节流进行OCR。可以通过这个接口识别 PIL Image 或者屏幕截图或者网络下载的图片，全程走内存，而无需先保存到硬盘。

**参数：** 

| 名称       | 默认值 | 类型  | 描述       |
| ---------- | ------ | ----- | ---------- |
| imageBytes | 必填   | bytes | 字节流对象 |

**返回值字典：同上** 

**示例：** 

```python
with open("test.png", 'rb') as f: # 获取图片字节流
    imageBytes = f.read() # 实际使用中，可以联网下载或者截图获取字节流
res = ocr.runBytes(imageBytes)
print("字节流识别结果：\n", res)
```

#### 4. 识别图片Base64编码字符串

**方法：** `runBase64()`

**说明：** 对一个Base64编码字符串进行OCR。

**参数：** 

| 名称        | 默认值 | 类型 | 描述               |
| ----------- | ------ | ---- | ------------------ |
| imageBase64 | 必填   | str  | Base64编码的字符串 |

**返回值字典：同上** 

#### 5. 格式化输出OCR结果

**方法：** `printResult()`

**说明：** 用于调试，打印一个OCR结果。

**参数：** 

| 名称 | 默认值 | 类型 | 描述              |
| ---- | ------ | ---- | ----------------- |
| res  | 必填   | dict | 一次OCR的返回结果 |

**无返回值** 

**示例：** 

```python
res = ocr.run("test.png")
print("格式化输出：")
ocr.printResult(res)
```

使用示例详见 [demo1.py](demo1.py)

### 第三步：关闭OCR引擎进程

一般情况下，在程序结束或者释放ocr对象时会自动关闭引擎子进程，无需手动管理。

如果希望手动关闭引擎进程，可以使用 `exit()` 方法。

**示例：** 

```python
ocr.exit()
```

如果需要更换识别语言，则重新创建ocr对象即可，旧的对象析构时也会自动关闭旧引擎进程。

**示例：** 

```python
argument = {'config_path': "语言1.txt"}
ocr = GetOcrApi(r"…………\PaddleOCR_json.exe", argument)
# TODO: 识别语言1

argument = {'config_path': "语言2.txt"}
ocr = GetOcrApi(r"…………\PaddleOCR_json.exe", argument)
# TODO: 识别语言2
```

# 结果可视化模块

纯Python实现，不依赖PPOCR引擎的C++ opencv可视化模块，避免中文兼容性问题。

需要PIL图像处理库：`pip install pillow`

```python
from PPOCR_visualize import visualize
```

### 获取文本块

首先得成功执行一次OCR，获取文本块列表（即`['data']`部分）
```python
testImg = "D:/test.png"
getObj = ocr.run(testImg)
if not getObj["code"] == 100:
    print('识别失败！！')
    exit()
textBlocks = getObj["data"]  # 提取文本块数据
```

### 展示结果图片

只需一行代码，传入文本块和原图片的路径，打开图片浏览窗口
```python
visualize(textBlocks, testImg).show()
```
此时程序阻塞，直到关闭图片浏览窗口才继续往下走。

### 图片保存到本地
```python
visualize(textBlocks, testImg).save('可视化结果.png')
```

### 获取PIL Image对象
```python
vis = visualize(textBlocks, testImg)
img = vis.get()
```

### 调整显示图层

以上`show`,`save`,`get`三个接口，均能开启或禁用指定图层：

- `isBox` T时启用包围盒图层。
- `isText` T时启用文字图层。
- `isOrder` T时启用序号图层。
- `isSource` T时启用原图。F禁用原图，即得到透明背景的纯可视化结果。

### 左右对比

传入两个PIL Image对象，返回它们左右拼接而成的新Image
```python
img_12 = visualize.createContrast(img1, img2)
```

### 调整显示效果（颜色、粗细、字体等）

导入PIL库，以便操作图片对象
```python
from PIL import Image
```

接口创建各个图层，传入文本块、要生成的图层大小、自定义参数，然后将各个图层合并

颜色有关的参数，均可传入6位RGB十六进制码（如`#112233`）或8位RGBA码（最后两位控制透明度，如`#11223344`）
```python
# 创建各图层
img = Image.open(testImg).convert('RGBA')  # 原始图片背景图层
imgBox = visualize.createBox(textBlocks, img.size,  # 包围盒图层
                             outline='#ccaa99aa', width=10)
imgText = visualize.createText(textBlocks, img.size,  # 文本图层
                               fill='#44556699')
# 合并各图层
img = visualize.composite(img, imgBox)
img = visualize.composite(img, imgText)
img.show() # 显示
```

使用示例详见 [demo2.py](demo2.py)

# 文本后处理 tbpu 

(text block processing unit)

```python
import tbpu
```

由 [Umi-OCR](https://github.com/hiroi-sora/Umi-OCR) 带来的技术。

OCR返回的结果中，一项包含文字、包围盒、置信度的元素，称为一个“文本块” - text block 。

文块不一定是完整的一句话或一个段落。反之，一般是零散的文字。一个OCR结果常由多个文块组成。文块后处理就是：将传入的多个文块进行处理，比如合并、排序、删除文块。

### 方案列表

- 优化单行: `run_merge_line_h`
- 合并多行-左对齐: `run_merge_line_h_m_left`
- 合并多行-自然段: `run_merge_line_h_m_paragraph`
- 合并多行-模糊匹配: `run_merge_line_h_m_fuzzy`
- 竖排-从左到右-单行: `run_merge_line_v_lr`
- 竖排-从右至左-单行: `run_merge_line_v_rl`

### 使用

向接口传入文本块列表（即`['data']`部分），返回新的文本块列表。
```python
# 执行文本块后处理：合并自然段
textBlocksNew = tbpu.run_merge_line_h_m_paragraph(textBlocks)
```
执行后，原列表 textBlocks 的结构可能被破坏，不要再使用原列表（或先深拷贝备份）。

可以串联使用，但目前这没什么意义。（除非你将Umi-OCR项目中的忽略区域模块分离出来，它跟段落合并串联使用相性良好。）
```python
tb1 = tbpu.run_merge_line_h_m_paragraph(textBlocks)
tb2 = tbpu.merge_line_h_m_fuzzy(tb1)
```

跟结果可视化配合使用：
```python
textBlocks = getObj["data"]  # 提取文本块数据

# OCR原始结果的可视化Image
img1 = visualize(textBlocks, testImg).get(isOrder=True)

# 执行文本块后处理：合并自然段
textBlocksNew = tbpu.run_merge_line_h_m_paragraph(textBlocks)

# 后处理结果的可视化Image
img2 = visualize(textBlocksNew, testImg).get(isOrder=True)

# 左右拼接图片并展示
visualize.createContrast(img1, img2).show()
```
![对比.png](https://tupian.li/images/2022/11/03/6363afbbb2fcd.png)

使用示例详见 [demo3.py](demo3.py)