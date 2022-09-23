# 调用 PaddleOCR-json.exe 的 Python Api
# 项目主页：
# https://github.com/hiroi-sora/PaddleOCR-json

import os
import subprocess  # 进程，管道
from json import loads as jsonLoads, dumps as jsonDumps
from sys import platform as sysPlatform  # popen静默模式


class PPOCR:
    """调用OCR"""

    def __init__(self, exePath: str, argument: dict = None):
        """初始化识别器。\n
        :exePath: 识别器`PaddleOCR_json.exe`的路径。\n
        :argument: 启动参数，字典`{"键":值}`。参数说明见 https://github.com/hiroi-sora/PaddleOCR-json
        """
        cwd = os.path.abspath(os.path.join(exePath, os.pardir))  # 获取exe父文件夹
        # 处理启动参数
        if not argument == None:
            for key, value in argument.items():
                if isinstance(value, str):  # 字符串类型的值加双引号
                    exePath += f' --{key}:"{value}"'
                else:
                    exePath += f' --{key}:{value}'
        if 'use_debug' not in exePath:  # 关闭debug模式
            exePath += ' --use_debug=0'
        # 设置子进程启用静默模式，不显示控制台窗口
        startupinfo = None
        if 'win32' in str(sysPlatform).lower():
            startupinfo = subprocess.STARTUPINFO()
            startupinfo.dwFlags = subprocess.CREATE_NEW_CONSOLE | subprocess.STARTF_USESHOWWINDOW
            startupinfo.wShowWindow = subprocess.SW_HIDE
        self.ret = subprocess.Popen(  # 打开管道
            exePath, cwd=cwd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            # stderr=subprocess.PIPE,
            startupinfo=startupinfo  # 开启静默模式
        )
        # 启动子进程
        while True:
            if not self.ret.poll() == None:  # 子进程已退出，初始化失败
                raise Exception(f'OCR init fail.')
            initStr = self.ret.stdout.readline().decode('utf-8', errors='ignore')
            if 'OCR init completed.' in initStr:  # 初始化成功
                break

    def run(self, imgPath: str, argument: dict = None):
        """对一张图片文字识别。\n
        :exePath: 图片路径。\n
        :argument: (可选)热更新参数，字典`{"键":值}`。参数说明见 https://github.com/hiroi-sora/PaddleOCR-json\n
        :return:  {'code': 识别码, 'data': 内容列表或错误信息字符串}\n"""
        if not self.ret.poll() == None:
            return {'code': 400, 'data': f'子进程已崩溃。'}
        writeDict = {'image_dir': imgPath}
        if argument:
            writeDict.update(argument)
        # 输入路径
        writeStr = jsonDumps(writeDict, ensure_ascii=True, indent=None)+'\n'
        try:
            self.ret.stdin.write(writeStr.encode('utf-8'))
            self.ret.stdin.flush()
        except Exception as e:
            return {'code': 400, 'data': f'向识别器进程写入图片地址失败，疑似子进程已崩溃。{e}'}
        # 获取返回值
        try:
            getStr = self.ret.stdout.readline().decode('utf-8', errors='ignore')
            # print(getStr)
        except Exception as e:
            return {'code': 401, 'data': f'读取识别器进程输出值失败，疑似传入了不存在或无法识别的图片 \"{imgPath}\" 。{e}'}
        try:
            return jsonLoads(getStr)
        except Exception as e:
            return {'code': 402, 'data': f'识别器输出值反序列化JSON失败，疑似传入了不存在或无法识别的图片 \"{imgPath}\" 。异常信息：{e}。原始内容：{getStr}'}

    def runClipboard(self):
        """立刻对剪贴板第一位的图片进行文字识别。"""
        return self.run('clipboard')

    def stop(self):
        self.ret.kill()  # 关闭子进程

    def __del__(self):
        self.stop()
