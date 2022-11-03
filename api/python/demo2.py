#    demo1.py ：演示OCR基础功能
# 👉 demo2.py ：演示可视化接口
#    demo3.py ：演示OCR文段后处理（段落合并）接口

from PPOCR_api import PPOCR
from PPOCR_visualize import visualize

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
ocr = PPOCR(r'…………\PaddleOCR_json.exe')
print(f'初始化OCR成功，进程号为{ocr.ret.pid}')

testImg = r'………\测试.png'

# OCR识别图片，获取文本块
getObj = ocr.run(testImg)
ocr.stop()  # 结束引擎子进程
if not getObj["code"] == 100:
    print('识别失败！！')
    exit()
textBlocks = getObj["data"]  # 提取文本块数据

# 可视化演示

# 传入文本块和图片路径，显示结果
print('显示图片！')
visualize(textBlocks, testImg).show()
# 程序阻塞，直到关闭图片浏览窗口才继续往下走。如果长时间不动，注释掉上面这行再跑

# 启用文本显示，图片保存到本地
print('保存图片！')
vis = visualize(textBlocks, testImg)
vis.save('可视化结果.png', isText=True)

print('获取图片！')
# 禁用包围盒，获取原图片的 PIL Image 对象
visImg1 = vis.get(isBox=False)
# 启用文本和序号、禁用原图（显示透明背景），获取 PIL Image 对象
visImg2 = vis.get(isText=True, isOrder=True, isSource=False)
# 显示两个图片的左右对比
visualize.createContrast(visImg1, visImg2).show()
# 接下来可以还用PIL库对visImg进一步处理。

print('程序结束。')
