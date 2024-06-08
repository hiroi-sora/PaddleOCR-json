#    demo1.py ：演示OCR基础功能
# 👉 demo2.py ：演示可视化接口
#    demo3.py ：演示OCR文段后处理（段落合并）接口

from PPOCR_api import GetOcrApi
from PPOCR_visualize import visualize

import os

# 测试图片路径
TestImagePath = os.path.join(os.path.dirname(os.path.abspath(__file__)), "test.jpg")

# 初始化识别器对象，传入 PaddleOCR-json 引擎路径。
ocr = GetOcrApi(r"Your Path/PaddleOCR-json.exe")

if ocr.getRunningMode() == "local":
    print(f"初始化OCR成功，进程号为{ocr.ret.pid}")
elif ocr.getRunningMode() == "remote":
    print(f"连接远程OCR引擎成功，ip：{ocr.ip}，port：{ocr.port}")
print(f"\n测试图片路径：{TestImagePath}")


# OCR识别图片，获取文本块
getObj = ocr.run(TestImagePath)
ocr.exit()  # 结束引擎子进程
if not getObj["code"] == 100:
    print("识别失败！！")
    exit()
textBlocks = getObj["data"]  # 提取文本块数据

# 可视化演示

# 示例1：传入文本块和图片路径，显示结果
print("显示图片！")
visualize(textBlocks, TestImagePath).show()
# 程序阻塞，直到关闭图片浏览窗口才继续往下走。如果长时间不动，注释掉上面这行再跑

# 示例2：显示更详细的信息
vis = visualize(textBlocks, TestImagePath)
print("获取图片！")
# 禁用包围盒，获取原图片的 PIL Image 对象
visImg1 = vis.get(isBox=False)
# 启用文本和序号、禁用原图（显示透明背景），获取 PIL Image 对象
visImg2 = vis.get(isText=True, isOrder=True, isSource=False)
# 获取两个图片的左右对比，左边是原图，右边是单独的文本框
vis = visualize.createContrast(visImg1, visImg2)
# 显示该对比
vis.show()
# 接下来可以还用PIL库对visImg进一步处理。

# 保存到本地
print(f"保存图片到 {os.path.dirname(os.path.abspath(__file__))}\\可视化结果.png ")
vis.save(f"{os.path.dirname(os.path.abspath(__file__))}\\可视化结果.png", isText=True)

print("程序结束。")
