# PaddleOCR-json python api 调用示例

from PPOCR_api import PPOCR

# 初始化，传入exe路径
ocr = PPOCR('D:\…………\PaddleOCR-json\PaddleOCR_json.exe')

# 识别图片
print("开始识别图片！")
getObj = ocr.run(r'D:\图片\test.png')
print(f"图片识别完毕，结果：\n{getObj}")
