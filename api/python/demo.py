from PPOCR_api import PPOCR

# 初始化识别器对象，传入 PaddleOCR_json.exe 的路径
ocr = PPOCR(r'…………\PaddleOCR_json.exe')
print(f'初始化OCR成功，进程号为{ocr.ret.pid}')

testImg = r'………\测试.png'

# 常规功能：识别图片，传入图片路径
getObj = ocr.run(testImg)
print(f'图片识别完毕，状态码：{getObj["code"]} 结果：\n{getObj["data"]}\n')
# print(getObj)

# 识别一张分辨率过大的图片，通过热更新调高压缩阈值limit_side_len，以获得更佳效果
# 建议对于大于4k的图片，压缩阈值可设为图片最大边长的一半，如高为7000的长截图，设为3500
testLongImg = r'………\大分辨率测试图.png'
updateDict = {'limit_side_len': 3500}
getObj = ocr.run(testLongImg, updateDict)
print(
    f'热更新并识图完毕，状态码：{getObj["code"]} 日志：\n{getObj["hotUpdate"]}\n结果：\n{getObj["data"]}\n')
# 完成后，调回正常的压缩阈值960，以免影响后续正常图片的识别效率
ocr.run('', argument={'limit_side_len': 960})

# 剪贴板识别功能
# 剪贴板中可以是位图（截图、网页复制），也可以是单个文件句柄（文件管理器中复制）。
getObj = ocr.runClipboard()
print(f'剪贴板识别完毕，状态码：{getObj["code"]} 结果：\n{getObj["data"]}\n')

# 启用结果可视化：在图片上绘制包围盒，保存到ocr_output目录下
updateDict = {'visualize': 1, 'output': 'D:/ocr_output/'}
getObj = ocr.run(testImg, updateDict)
print(
    f'图片识别完毕，状态码：{getObj["code"]} 日志：\n{getObj["hotUpdate"]}\n结果：\n{getObj["data"]}\n')

ocr.stop()  # 结束引擎子进程。
print('程序结束。')
