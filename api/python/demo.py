from PPOCR_api import PPOCR

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
ocr = PPOCR('D:\…………\PaddleOCR-json\PaddleOCR_json.exe')

# 识别图片，传入图片路径
while True:
    imgPath = input('请输入图片路径，退出直接回车：')
    if imgPath:
        getObj = ocr.run(imgPath)
        print(f'图片识别完毕，状态码：{getObj["code"]} 结果：\n{getObj["data"]}\n')
    else:
        break

del ocr  # 销毁识别器对象，结束子进程。
print('程序结束。')
