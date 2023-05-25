# 👉 demo1.py ：演示OCR基础功能
#    demo2.py ：演示可视化接口
#    demo3.py ：演示OCR文段后处理（段落合并）接口

from PPOCR_api import GetOcrApi

import os

# 测试图片路径
TestImagePath = f"{os.path.dirname(os.path.abspath(__file__))}\\test.jpg"

# 初始化识别器对象，传入 PaddleOCR-json.exe 的路径。请改成你自己的路径
ocr = GetOcrApi(r"D:\MyCode\CppCode\PaddleOCR-json\cpp\build\Release\PaddleOCR-json.exe")
print(f'初始化OCR成功，进程号为{ocr.ret.pid}')
print(f'\n测试图片路径：{TestImagePath}')

# 示例1：识别本地图片
res = ocr.run(TestImagePath)
print(f'\n示例1-图片路径识别结果（原始信息）：\n{res}')
print(f'\n示例1-图片路径识别结果（格式化输出）：')
ocr.printResult(res)

# 示例2：识别剪贴板图片
res = ocr.runClipboard()
if res["code"] == 212:
    print(f'\n示例2-当前剪贴板中没有图片。')
else:
    print(f'\n示例2-剪贴板识别结果：')
    ocr.printResult(res)

# 示例3：识别图片字节流
with open(TestImagePath, 'rb') as f: # 获取图片字节流
    imageBytes = f.read() # 实际使用中，可以联网下载或者截图获取字节流，直接送入OCR，无需保存到本地中转。
res = ocr.runBytes(imageBytes)
print(f'\n示例3-字节流识别结果：')
ocr.printResult(res)

# 示例4：识别 PIL Image 对象
try:
    from PIL import Image
    from io import BytesIO
except Exception:
    print("安装Pillow库后方可测试示例4。") 
    Image = None
if Image:
    # 创建一个PIL Image对象
    pilImage = Image.open(TestImagePath) 
    # Image 对象转为 字节流
    buffered = BytesIO() 
    pilImage.save(buffered, format="JPEG")
    # 送入OCR
    res = ocr.runBytes(imageBytes)
    print(f'\n示例4-PIL Image 识别结果：')
    ocr.printResult(res)