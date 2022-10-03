# PaddleOCR-json-node-api
基于[hiroi-sora/PaddleOCR-json](https://github.com/hiroi-sora/PaddleOCR-json)的node.js api.

<details>
<summary>Log</summary>

v1.0.4 2022.10.1

适配[hiroi-sora/PaddleOCR-json v1.2.1](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/v1.2.1).

不使用`iconv-lite`包.

更改启动参数输入方式.

v1.0.3 2022.10.1

适配[hiroi-sora/PaddleOCR-json v1.2.1](https://github.com/hiroi-sora/PaddleOCR-json/releases/tag/v1.2.1).

v1.0.2 2022.9.14

增加环境选项.

v1.0.1 2022.9.14

修复无法识别 Alpha版 的启动完成标志的bug.
JSON输入更改为ascii转义.

v1.0.0 2022.9.10

</details>

## 快速开始

### OCR api

将本项目引入(或单独引入`OCR.js`)至你的项目中, 引用OCR.

```js
const OCR = require('./OCR');
const ocr = new OCR('PaddleOCR_json.exe', [], {
    cwd: './PaddleOCR-json',
}, false);

ocr.postMessage({ image_dir: 'path/to/test/img' })
    .then((data) => console.log(data));
    .then(() => ocr.terminate());
```

### server服务

```
npm i
npm start
```
打开 http://127.0.0.1:3000 测试

## api

### OCR

`OCR`是`worker_threads.Worker`的派生类.

#### new OCR(path, args, options, debug)

```js
const OCR = require('./OCR');
const ocr = new OCR('PaddleOCR_json.exe', [], {
    cwd: './PaddleOCR-json',
}, false);
```

`args`详见[Node.js child_process.spawn](https://nodejs.org/api/child_process.html#child_processspawncommand-args-options)与[hiroi-sora/PaddleOCR-json#配置参数说明](https://github.com/hiroi-sora/PaddleOCR-json#%E9%85%8D%E7%BD%AE%E5%8F%82%E6%95%B0%E8%AF%B4%E6%98%8E).

`options`详见[Node.js child_process.spawn](https://nodejs.org/api/child_process.html#child_processspawncommand-args-options).

#### OCR.postMessage(obj)

```js
ocr.postMessage({
    limit_side_len: 960,
    image_dir: 'path/to/test/img',
});
```
`ocr.postMessage`返回的是`Promise`对象.

`obj`详见[hiroi-sora/PaddleOCR-json#动态参数](https://github.com/hiroi-sora/PaddleOCR-json#%E5%8A%A8%E6%80%81%E5%8F%82%E6%95%B0)与[hiroi-sora/PaddleOCR-json/blob/main/docs/详细使用指南.md#-通过管道传json建议](https://github.com/hiroi-sora/PaddleOCR-json/blob/main/docs/%E8%AF%A6%E7%BB%86%E4%BD%BF%E7%94%A8%E6%8C%87%E5%8D%97.md#-%E9%80%9A%E8%BF%87%E7%AE%A1%E9%81%93%E4%BC%A0json%E5%BB%BA%E8%AE%AE)

与[hiroi-sora/PaddleOCR-json/blob/main/docs/详细使用指南.md#方式ex剪贴板识图](https://github.com/hiroi-sora/PaddleOCR-json/blob/main/docs/%E8%AF%A6%E7%BB%86%E4%BD%BF%E7%94%A8%E6%8C%87%E5%8D%97.md#%E6%96%B9%E5%BC%8Fex%E5%89%AA%E8%B4%B4%E6%9D%BF%E8%AF%86%E5%9B%BE)不同, 若`obj.image_dir`为`null`, 则获取剪贴板.

#### OCR.flush(obj)

`ocr.flush`是`ocr.postMessage`的别名.

#### 其他

你可以用`worker_threads.Worker`的api来监听或操作`OCR`实例.

例如:
```js
const ocr = new OCR();
ocr.on('message', (data) => {
    console.log(data);
    // 首先会输出 OCR init 的数据, 
    // 值为 { code: 0, message: 'OCR init completed.', pid: PID } .
    // PID 是进程 PaddleOCR-json.exe 的 pid值.
});
```

## License

```
        DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  1. You just DO WHAT THE FUCK YOU WANT TO.
```
