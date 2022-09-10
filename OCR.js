const { Worker, isMainThread, parentPort } = require('worker_threads');
if (isMainThread) {
    const { resolve: path_resolve } = require('path');
    class Queue extends Array {
        #status = false
        start() {
            if (this.#status) return false;
            this.#status = true;
            this.shift();
            return true;
        }
        close() {
            if (!this.#status) return false;
            this.#status = false;
            return true;
        }
        shift() {
            this.length && Promise.resolve(this[0]()).then(() => (super.shift(), this.#status && this.shift()));
        }
        push(cb) {
            super.push(cb) - 1 || this.#status && this.shift();
        }
    }
    class OCR extends Worker {
        #queue
        constructor() {
            super(__filename);
            this.#queue = new Queue();
            super.once('message', (code) => {
                console.log(code);
                this.#queue.start();
            });
        }
        get length() {
            return this.#queue.length;
        }
        postMessage(obj) {
            return new Promise((res) => {
                const queue = this.#queue;
                obj = Object.assign({}, obj);
                obj.image_dir &&= path_resolve('./', obj.image_dir);
                queue.push(() => new Promise((res_) => {
                    super.once('message', (data) => (res({
                        code: data.code,
                        message: data.code - 100 ? data.data : '',
                        data: data.code - 100 ? null : data.data,
                    }), res_()));
                    super.postMessage(obj);
                }));
            });
        }
        flush(obj) {
            return this.postMessage(obj);
        }
    }
    module.exports = OCR;
} else {
    const iconv = require('iconv-lite');
    function decode(data, encoding = 'cp936') {
        return iconv.decode(Buffer.from(data, 'binary'), encoding);
    }
    function encode(str, encoding = 'gbk') {
        return iconv.encode(str, encoding);
    }
    const { spawn } = require('child_process');
    new Promise((res) => {
        const proc = spawn('PaddleOCR_json.exe', [], {
            cwd: './PaddleOCR-json',
            encoding: 'buffer',
        });
        const initTag = decode(Buffer.from([79, 67, 82, 32, 105, 110, 105, 116, 32, 99, 111, 109, 112, 108, 101, 116, 101, 100, 46, 13, 10]));
        proc.stdout.on('data', function stdout(chunk) {
            if (decode(chunk) !== initTag) return;
            proc.stdout.off('data', stdout);
            return res(proc);
        });
        proc.stderr.on('data', (data) => {
            // console.log(iconvDecode(data));
        });
        proc.on('close', (code) => {
            console.log('close code: ', code);
        });
        proc.on('exit', (code) => {
            console.log('exit code: ', code);
        });
    }).then((proc) => {
        parentPort.postMessage({
            code: 0,
            message: 'OCR init completed.',
            pid: proc.pid,
        });
        parentPort.on('message', (data) => {
            proc.stdin.write(encode(`${JSON.stringify(data)}\n`))
        });
        proc.stdout.on('data', (chunk) => {
            parentPort.postMessage(JSON.parse(decode(chunk, 'utf-8')));
        });
    });
}