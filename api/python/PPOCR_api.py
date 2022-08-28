# from asyncio.windows_events import NULL
import os
import subprocess  # 进程，管道
from sys import platform as sysPlatform  # popen静默模式
from json import loads as jsonLoads


class PPOCR:
    """调用OCR"""

    def __init__(self, exePath):
        """初始化识别器。\n
        传入识别器exe路径"""
        cwd = os.path.abspath(os.path.join(exePath, os.pardir))  # 获取exe父文件夹
        # exePath += ' 这里可以添加参数'
        startupinfo = None  # 静默模式设置
        if 'win32' in str(sysPlatform).lower():
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags = subprocess.CREATE_NEW_CONSOLE | subprocess.STARTF_USESHOWWINDOW
            startupinfo.wShowWindow = subprocess.SW_HIDE
        self.ret = subprocess.Popen(  # 打开管道
            exePath, cwd=cwd,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            startupinfo=startupinfo  # 开启静默模式
        )
        maxWaitLine = 1000  # 防死循环
        errorFlags = ['ERROR', 'Error', 'OCR exit.']  # 报错标志
        initMsg = ""
        while(maxWaitLine > 0):
            initStr = self.ret.stdout.readline().decode('gbk', errors='ignore')
            initMsg += initStr
            if "OCR init completed." in initStr:  # 初始化成功
                break
            for flag in errorFlags:
                if flag in initStr:  # 遇到报错，初始化失败
                    print(f"初始化OCR失败！日志：\n{initMsg}")
                    return
            maxWaitLine -= 1
        if maxWaitLine <= 0:
            print(f"初始化OCR失败！日志：[{initMsg}]\n")
            print(initMsg)
        else:
            print("初始化OCR成功！")

    def run(self, imgPath):
        """对一张图片文字识别。
        输入图片路径。\n
        识别成功时，返回列表，每项是一组文字的信息。\n
        识别失败时，返回字典 {error:异常信息，text:(若存在)原始识别字符串} 。"""
        if not imgPath[-1] == "\n":
            imgPath += "\n"
        try:
            self.ret.stdin.write(imgPath.encode("gbk"))
            self.ret.stdin.flush()
        except Exception as e:
            return {"code": 400, "data": f"向识别器进程写入图片地址失败，疑似该进程已崩溃。{e}"}
        try:
            getStr = self.ret.stdout.readline().decode('utf-8', errors='ignore')
            print(getStr)
        except Exception as e:
            if imgPath[-1] == "\n":
                imgPath = imgPath[:-1]
            return {"code": 401, "data": f"读取识别器进程输出值失败，疑似传入了不存在或无法识别的图片 \"{imgPath}\" 。{e}"}
        try:
            return jsonLoads(getStr)
        except Exception as e:
            if imgPath[-1] == "\n":
                imgPath = imgPath[:-1]
            return {"code": 402, "data": f"识别器输出值反序列化JSON失败，疑似传入了不存在或无法识别的图片 \"{imgPath}\" 。异常信息：{e}。原始内容：{getStr}"}

    def __del__(self):
        self.ret.kill()  # 关闭子进程
        print("关闭OCR子进程！")
