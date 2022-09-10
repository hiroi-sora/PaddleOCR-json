# PaddleOCR-json-node-api
基于[hiroi-sora/PaddleOCR-json](https://github.com/hiroi-sora/PaddleOCR-json)的node.js api.

## 快速开始

### OCR api

将本项目(或此项目中的`PaddleOCR-json`与`OCR.js`)引入至你的项目中, 引用OCR.

```js
const OCR = require('./OCR');
const ocr = new OCR();

ocr.postMessage({ image_dir: 'path/to/test/img' })
    .then((data) => console.log(data));
    .then(() => ocr.terminate());
```

### server服务

```
npm i
npm start
```

## api

### OCR

`OCR`是`worker_threads.Worker`的派生类.

#### new OCR

```js
const config = {}
const ocr = new OCR(config)
```

`config`详见[hiroi-sora/PaddleOCR-json#4-注入配置参数](https://github.com/hiroi-sora/PaddleOCR-json#4-%E6%B3%A8%E5%85%A5%E9%85%8D%E7%BD%AE%E5%8F%82%E6%95%B0)和[hiroi-sora/PaddleOCR-json#5-配置信息说明](https://github.com/hiroi-sora/PaddleOCR-json#5-%E9%85%8D%E7%BD%AE%E4%BF%A1%E6%81%AF%E8%AF%B4%E6%98%8E)的热更新配置.

#### OCR.postMessage

```js
ocr.postMessage(obj);
ocr.postMessage({
    limit_side_len: 960,
    image_dir: 'path/to/test/img',
});
```
`ocr.postMessage`返回的是`Promise`对象.

`obj`详见[hiroi-sora/PaddleOCR-json#5-配置信息说明](https://github.com/hiroi-sora/PaddleOCR-json#5-%E9%85%8D%E7%BD%AE%E4%BF%A1%E6%81%AF%E8%AF%B4%E6%98%8E)

#### OCR.flush

`ocr.flush`是`ocr.postMessage`的别名.

### 其他

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

>本项目的`js`代码均以WTFPL条款开源.
>`PaddleOCR-json`部分详见[hiroi-sora/PaddleOCR-json](https://github.com/hiroi-sora/PaddleOCR-json).

For js code part:
