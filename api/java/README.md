# PaddleOCR-json-java-api

[PaddleOCR-json](https://github.com/hiroi-sora/PaddleOCR-json) 的简单 Java 封装。

## 依赖
- Gson
- Java 8 或更新版本

## 使用
1. 在项目中引入 'Ocr.java'
2. 参考如下代码片段调用 OCR (或参考完整示例 [Main.java](https://github.com/jerrylususu/PaddleOCR-json-java-api/blob/main/src/main/java/org/example/Main.java))

```java
// paddleocr_json 的可执行文件所在路径
String exePath = "path/to/executable";

// 可选的配置项
Map<String, Object> arguments = new HashMap<>();
// arguments.put("use_angle_cls", true);

// 初始化 OCR
try (Ocr ocr = new Ocr(new File(exePath), arguments)) {

    // 对一张图片进行 OCR
    String imgPath = "path/to/img";
    OcrResponse resp = ocr.runOcr(new File(imgPath));
    
    // 或者直接识别剪贴板中的图片
    // OcrResponse resp = ocr.runOcrOnClipboard();
    
    // 读取结果
    if (resp.code == OcrCode.OK) {
        for (OcrEntry entry : resp.data) {
            System.out.println(entry.text);
        }
    } else {
        System.out.println("error: code=" + resp.code + " msg=" + resp.msg);
    }
} catch (IOException e) {
    e.printStackTrace();
}
```

## 参考
- Unicode 转换为 ASCII 序列: [EscapedWriter (Soot Project)](https://github.com/soot-oss/soot/blob/3966f565db6dc2882c3538ffc39e44f4c14b5bcf/src/main/java/soot/util/EscapedWriter.java)
