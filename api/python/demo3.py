#    demo1.py ：演示OCR基础功能
#    demo2.py ：演示可视化接口
# 👉 demo3.py ：演示OCR文段后处理（段落合并）接口

from PPOCR_api import PPOCR
from PPOCR_visualize import visualize  # 可视化
import tbpu

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

img1 = visualize(textBlocks, testImg).get(isOrder=True)  # OCR原始结果的可视化Image

# 执行文本块后处理：合并自然段
# 传入OCR结果列表，返回新的文本块列表
textBlocksNew = tbpu.run_merge_line_h_m_paragraph(textBlocks)
# 注意，处理后原列表 textBlocks 的结构可能被破坏，不要再使用原列表（或先深拷贝备份）。

# 可视化
img2 = visualize(textBlocksNew, testImg).get(isOrder=True)  # 后处理结果的可视化Image
visualize.createContrast(img1, img2).show()  # 左右拼接图片并展示
