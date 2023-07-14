# 将 PaddleOCR-json 结果可视化表现
# 项目主页：
# https://github.com/hiroi-sora/PaddleOCR-json
from PIL import Image, ImageDraw, ImageFont
import math


class visualize:
    '''可视化'''

    # ================================ 静态方法 ================================

    @staticmethod
    def createBox(textBlocks, size,
                  fill='#00500040', outline='#11ff22', width=6):
        '''创建包围盒图层，返回PIL Image对象。\n
        :textBlocks: 文本块列表。\n
        :size: 图片尺寸。\n
        以下为可选字段：（颜色为十六进制6位RGB或8位RGBA字符串，如 #112233ff）\n
        :fill: 包围盒填充颜色。\n
        :outline: 包围盒轮廓颜色。\n
        :width: 包围盒轮廓粗细，像素。
        '''
        img = Image.new('RGBA', size, 0)
        draw = ImageDraw.Draw(img)
        for tb in textBlocks:
            box = [tuple(tb['box'][0]), tuple(tb['box'][1]),
                   tuple(tb['box'][2]), tuple(tb['box'][3])]
            draw.polygon(box, fill=fill, outline=outline, width=width)
        return img

    @staticmethod
    def createText(textBlocks, size,
                   ttfPath='C:\Windows\Fonts\msyh.ttc', ttfScale=0.9,
                   fill='#ff0000'):
        '''创建文字图层，返回PIL Image对象。\n
        :textBlocks: 文本块列表。\n
        :size: 图片尺寸。\n
        以下为可选字段：\n
        :ttfPath: 字体文件路径。默认为微软雅黑，若不存在此字体会报错。\n
        :ttfScale: 字体大小整体缩放系数，应在1附近。\n
        :fill: 文字颜色，十六进制6位RGB或8位RGBA字符串，如 #112233ff。\n
        '''
        img = Image.new('RGBA', size, 0)
        draw = ImageDraw.Draw(img)
        ttfDict = {}  # 缓存不同大小的字体对象
        for tb in textBlocks:
            text = tb['text']
            xy = tuple(tb['box'][0])  # 左上角坐标
            xy1 = tb['box'][3]  # 左下角坐标# 行高
            hight = round(
                math.sqrt(((xy[0]-xy1[0])**2)+((xy[1]-xy1[1])**2))*ttfScale)
            if hight not in ttfDict:
                ttfDict[hight] = ImageFont.truetype(
                    ttfPath, hight)  # 创建新大小的字体
            draw.text(xy, text, font=ttfDict[hight], fill=fill)
        return img

    @staticmethod
    def createOrder(textBlocks, size,
                    ttfPath='C:\Windows\Fonts\msyh.ttc', ttfSize=50,
                    fill='#2233ff', bg='#ffffffe0'):
        '''创建序号图层，返回PIL Image对象。\n
        :textBlocks: 文本块列表。\n
        :size: 图片尺寸。\n
        以下为可选字段：\n
        :ttfPath: 字体文件路径。默认为微软雅黑，若不存在此字体会报错。\n
        :ttfSize: 字体大小。\n
        :fill: 文字颜色，十六进制6位RGB或8位RGBA字符串，如 #112233ff。\n
        '''
        img = Image.new('RGBA', size, 0)
        draw = ImageDraw.Draw(img)
        ttf = ImageFont.truetype(ttfPath, ttfSize)  # 字体
        for index, tb in enumerate(textBlocks):
            text = f'{index+1}'
            xy = tuple(tb['box'][0])  # 左上角坐标
            x_, y_, w, h = ttf.getbbox(text)  # 获取宽高。只需要w和h
            w *= 1.1
            h *= 1.1
            draw.rectangle((xy, (xy[0]+w, xy[1]+h)), fill=bg, width=0)  # 背景矩形
            draw.text(xy, text, font=ttf, fill=fill)  # 文字
        return img

    @staticmethod
    def createContrast(img1, img2):
        '''左右拼合两个图片，创建对比图层，返回PIL Image对象。'''
        size = (img1.size[0]+img2.size[0], max(img1.size[1], img2.size[1]))
        img = Image.new('RGBA', size, 0)
        img.paste(img1, (0, 0))
        img.paste(img2, (img1.size[0], 0))
        return img

    @staticmethod
    def composite(img1, img2):
        '''传入两个PIL Image对象（RGBA格式），以img1为底，将img2叠加在其上
        返回生成的图片'''
        return Image.alpha_composite(img1, img2)

    # ================================ 快捷接口 ================================

    def __init__(self, textBlocks, imagePath):
        """创建可视化对象。\n
        :textBlocks: 文本块列表，即OCR返回的data部分\n
        :imagePath: 对应的图片路径。
        """
        self.imgSource = Image.open(imagePath).convert('RGBA')  # 原始图片图层
        self.size = self.imgSource.size
        self.imgBox = self.createBox(textBlocks, self.size)  # 包围盒图层
        self.imgText = self.createText(textBlocks, self.size)  # 文字图层
        self.imgOrder = self.createOrder(textBlocks, self.size)  # 序号图层

    def get(self, isBox=True, isText=False, isOrder=False, isSource=True):
        '''返回合成可视化结果的PIL Image图像。\n
        :isBox: T时返回包围盒图层。\n
        :isText: T时返回文字图层。\n
        :isOrder: T时返回序号图层。\n
        :isSource: T时返回原图。F时返回透明背景的纯可视化结果。\n
        '''
        img = Image.new('RGBA', self.size, 0)
        flags = (isSource, isBox, isText, isOrder)
        for index, im in enumerate([self.imgSource, self.imgBox, self.imgText, self.imgOrder]):
            if im and flags[index]:
                img = visualize.composite(img, im)
        return img

    def show(self, isBox=True, isText=False, isOrder=False, isSource=True):
        '''显示可视化结果图像。\n
        :isBox: T时返回包围盒图层。\n
        :isText: T时返回文字图层。\n
        :isOrder: T时返回序号图层。\n
        :isSource: T时返回原图。F时返回透明背景的纯可视化结果。\n
        '''
        img = self.get(isBox, isText, isOrder, isSource)
        img.show()

    def save(self, path='', isBox=True, isText=False, isOrder=False, isSource=True):
        '''保存可视化结果图像。\n
        :path: 保存路径。\n
        :isBox: T时返回包围盒图层。\n
        :isText: T时返回文字图层。\n
        :isOrder: T时返回序号图层。\n
        :isSource: T时返回原图。F时返回透明背景的纯可视化结果。\n
        '''
        img = self.get(isBox, isText, isOrder, isSource)
        img.save(path)