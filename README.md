# PaddleOCR-json 图片转文字程序

图片批量离线OCR文字识别程序。输入图片路径，输出识别结果json字符串，方便别的程序调用。

**本程序的GUI形式：[Umi-OCR 批量图片转文字工具](https://github.com/hiroi-sora/Umi-OCR)**


## 开始使用

下载 [PaddleOCR-json v1.0](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/v1.0) 并解压，即可。

## 命令行试用

直接打开`PaddleOcr_json.exe`。输入图片路径，回车。
- 任务成功时，输出图片识别信息的json（数组）。
- 任务失败时，输出json为：{"error":"异常原因xxxx"} 。
- 输出一条信息后，可继续接收路径输入；程序是死循环。
- 支持输入带空格的路径。
- 命令行模式下仅支持英文路径。

程序接受`gbk`编码输入，而输出是`utf-8`。因此，命令行模式下，无法识别输入的非英文字符。

## python调用

- 通过管道与识别器程序交互。
- 支持中文路径：将含中文字符串编码为`gbk`输入管道，即可被正确识别。

#### 简单例子

```python
import subprocess
import json

imgPath = "测试文件夹\\测试图片.png" # 待检测图片路径，支持中文

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
```

#### 实用例子

[PaddleOCR-demo.py](PaddleOCR-demo.py)

这里代码可能与下载的`PaddleOCR-json.rar`中的不一样，以这里为准。

## 输出值JSON说明

`PaddleOcr_json.exe` 将把识别信息以json格式字符串的形式打印到控制台。
正常情况下，输出值为 **数组** ，数组中每一项含三个元素：
- `text` ：文本内容，字符串。
- `box` ：文本包围盒，长度为8的数组，分别为左上角xy、右上角xy、右下角xy、左下角xy。整数。
- `score` ：识别置信度，浮点数。

## 载入多国语言语言

以法文为例：

1. 前往 [PP-OCR系列 多语言识别模型列表](https://gitee.com/paddlepaddle/PaddleOCR/blob/release/2.4/doc/doc_ch/models_list.md#23-%E5%A4%9A%E8%AF%AD%E8%A8%80%E8%AF%86%E5%88%AB%E6%A8%A1%E5%9E%8B%E6%9B%B4%E5%A4%9A%E8%AF%AD%E8%A8%80%E6%8C%81%E7%BB%AD%E6%9B%B4%E6%96%B0%E4%B8%AD) 下载对应的 **推理模型**`french_mobile_v2.0_rec_infer.tar` 和 **字典文件**`french_dict.txt`。
2. 在`PaddleOCR_Green`目录下创建文件夹`rec_fr`，将解压后的三个模型文件放进去。字典文件可直接放在目录下。
3. 复制一份识别器`PaddleOCR_json.exe`，命名为`PaddleOCR_json_fr.exe`
4. 复制一份配置单`PaddleOCR_json_config.txt`，命名为`PaddleOCR_json_fr_config.txt`
5. 打开配置单`PaddleOCR_json_fr_config.txt`，将`# rec config`相关的两个配置项改为：
    ```sh
    # rec config
    rec_model_dir  rec_fr
    char_list_file french_dict.txt
    ```
6. 保存文件，打开软件，将 **识别器路径** 改为 `PaddleOCR_Green\PaddleOCR_json_fr.exe`。

## 切换模型库

软件附带的模型可能过时，可以重新下载PaddleOCR的最新官方模型，享受更精准的识别质量。或者使用自己训练的模型。

#### 1. 下载模型
 - 前往[PaddleOCR](https://gitee.com/paddlepaddle/PaddleOCR#pp-ocr%E7%B3%BB%E5%88%97%E6%A8%A1%E5%9E%8B%E5%88%97%E8%A1%A8%E6%9B%B4%E6%96%B0%E4%B8%AD)下载一组推理模型（非训练模型）。**中英文超轻量PP-OCRv2模型** 体积小、速度快，**中英文通用PP-OCR server模型** 体积大、精度高。一般来说，轻量模型的精度已经非常不错，无需使用标准模型。
 - 注意只能使用v2.x版模型，不能使用github上[PaddlePaddle/PaddleOCR](https://github.com/PaddlePaddle/PaddleOCR)的v1.1版。v1版模型每个压缩包内有model和params两个文件，v2版则是每组三个文件；不能混用。

#### 2. 放置模型
- 将下载下来的方向分类器（如`ch_ppocr_mobile_v2.0_cls_infer.tar`）、检测模型（如`ch_PP-OCRv2_det_infer.tar`）、识别模型（如`ch_PP-OCRv2_rec_infer.tar`）解压，将文件分别放到对应文件夹 `cls、det、rec`。
- 打开PaddleOcr_json.exe。若无报错，则模型文件已正确加载。“Active code page: 65001”是正常现象。

#### 3. 调整配置
- `[exe名称]_config.txt`是全局配置文件，可设置模型位置、识别参数、开启GPU等。具体参考[官方文档](https://gitee.com/paddlepaddle/PaddleOCR/blob/release/2.4/doc/doc_ch/config.md)。
- 如果修改了exe名称，也需要同步修改配置文件名的前缀。

## 开发说明

### 部署项目

1. 下载并编译飞桨PaddleOCR C++预测库，可参考 [这篇教程](https://cloud.tencent.com/developer/article/1843504) 。
2. 将本项目`src`、`include`目录中的文件替换进去。
