# 👉 demo1.py ：演示OCR基础功能
#    demo2.py ：演示可视化接口
#    demo3.py ：演示OCR文段后处理（段落合并）接口

from PPOCR_api import GetOcrApi

import os

# 测试图片路径
TestImagePath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test.jpg")

# 初始化识别器对象，传入 PaddleOCR-json 引擎路径。
# 引擎下载地址： https://github.com/hiroi-sora/PaddleOCR-json/releases
# Windows： 传入 PaddleOCR-json.exe 的路径。
# Linux： 传入 run.sh 的路径
ocr = GetOcrApi(r"Your Path/PaddleOCR-json.exe")

if ocr.getRunningMode() == "local":
    print(f"初始化OCR成功，进程号为{ocr.ret.pid}")
elif ocr.getRunningMode() == "remote":
    print(f"连接远程OCR引擎成功，ip：{ocr.ip}，port：{ocr.port}")
print(f"\n测试图片路径：{TestImagePath}")

# 示例1：识别本地图片
res = ocr.run(TestImagePath)
print(f"\n示例1-图片路径识别结果（原始信息）：\n{res}")
print(f"\n示例1-图片路径识别结果（格式化输出）：")
ocr.printResult(res)

# 示例2：识别图片字节流
with open(TestImagePath, "rb") as f:  # 获取图片字节流
    # 实际使用中，可以联网下载或者截图获取字节流，直接送入OCR，无需保存到本地中转。
    imageBytes = f.read()
res = ocr.runBytes(imageBytes)
print(f"\n示例2-字节流识别结果：")
ocr.printResult(res)

# 示例3：识别 PIL Image 对象
try:
    from PIL import Image
    from io import BytesIO
except Exception:
    print("安装Pillow库后方可测试示例3。")
    Image = None
if Image:
    # 创建一个PIL Image对象
    pilImage = Image.open(TestImagePath)
    # Image 对象转为 字节流
    buffered = BytesIO()
    pilImage.save(buffered, format="PNG")
    imageBytes = buffered.getvalue()
    # 送入OCR
    res = ocr.runBytes(imageBytes)
    print(f"\n示例3-PIL Image 识别结果：")
    ocr.printResult(res)

# 以下示例默认禁用
# 示例4：识别剪贴板图片
if ocr.isClipboardEnabled():
    res = ocr.runClipboard()
    if res["code"] == 212:
        print(f"\n示例4-当前剪贴板中没有图片。")
    else:
        print(f"\n示例4-剪贴板识别结果：")
        ocr.printResult(res)
