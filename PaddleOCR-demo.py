import subprocess
from sys import platform as sysPlatform  # popen静默模式
from json import loads as jsonLoads


class OCR:

    def __init__(self, exePath, cwd=None):
        """初始化识别器。\n
        传入识别器exe路径，子进程目录(exe父目录)"""
        startupinfo = None  # 静默模式设置
        if 'win32' in str(sysPlatform).lower():
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags = subprocess.CREATE_NEW_CONSOLE | subprocess.STARTF_USESHOWWINDOW
            startupinfo.wShowWindow = subprocess.SW_HIDE
        self.ret = subprocess.Popen(  # 打开管道
            exePath,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            startupinfo=startupinfo  # 开启静默模式
        )
        self.ret.stdout.readline()  # 读掉第一行

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
            return {"error": f"向识别器进程写入图片地址失败，疑似该进程已崩溃。{e}"}
        try:
            getStr = self.ret.stdout.readline().decode('utf-8', errors='ignore')
        except Exception as e:
            if imgPath[-1] == "\n":
                imgPath = imgPath[:-1]
            return {"error": f"读取识别器进程输出值失败，疑似传入了不存在或无法识别的图片【{imgPath}】。{e}"}
        try:
            # 走到这里不一定代表识别成功，可能c++内部已经处理了异常，getStr本身就是 {"error":"xxxxx"}
            return jsonLoads(getStr)
        except Exception as e:
            if imgPath[-1] == "\n":
                imgPath = imgPath[:-1]
            return {"error": f"识别器输出值反序列化JSON失败，疑似传入了不存在或无法识别的图片【{imgPath}】。{e}", "text": getStr}

    def __del__(self):
        self.ret.kill()  # 关闭子进程


if __name__ == "__main__":
    oCN = OCR("PaddleOCR_json.exe")  # 默认中文识别器
    oJP = OCR("PaddleOCR_json_jp.exe")  # 日文识别器
    while True:
        p = input("请输入图片路径：\n")
        if input("本图片识别为中文（敲回车）还是日文（敲1）：") == "":
            dic = oCN.run(p)
        else:
            dic = oJP.run(p)
        if isinstance(dic, dict):
            print("识别失败，原因：", dic["error"])
            if "text" in dic:
                print("原始数据：\n", dic["text"])
        else:
            for i in dic:
                print(f'''
{i["text"]}
    左上角：{i["box"][0]},{i["box"][1]}
    右下角：{i["box"][4]},{i["box"][5]}
    置信度：{i["score"]}
    ''')
