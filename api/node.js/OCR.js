const { isMainThread } = require('worker_threads');
const { resolve: path_resolve } = require('path');
if (isMainThread) {
    const { Worker } = require('worker_threads');
    class Queue extends Array {
        constructor() {
            super();
            this.status = false
        }
        shift() {
            this.length && Promise.resolve(this[0]()).then(() => (super.shift(), this.status && this.shift()));
        }
        push(cb) {
            super.push(cb) - 1 || this.status && this.shift();
        }
    }
    class OCR extends Worker {
        #queue
        constructor(config = null, {
            path = 'PaddleOCR_json.exe',
            cwd = './PaddleOCR-json',
            debug = false,
        } = {}) {
            super(__filename, {
                workerData: {
                    path,
                    cwd,
                    debug,
                },
            });
            this.#queue = new Queue();
            if (config) this.postMessage(config);
            super.once('message', (code) => {
                console.log(code);
                this.#queue.status = true;
                this.#queue.shift();
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
    const { parentPort, workerData } = require('worker_threads');
    const iconv = require('iconv-lite');
    function decode(data, encoding = 'cp936') {
        return iconv.decode(Buffer.from(data, 'binary'), encoding);
    }
    function encode(str, encoding = 'ascii') {
        return iconv.encode(str, encoding);
    }
    const { spawn } = require('child_process');
    new Promise((res) => {
        const {
            path = 'PaddleOCR_json.exe',
            cwd = './PaddleOCR-json',
            debug = false,
        } = workerData;
        const proc = spawn(path_resolve(cwd, path), [], {
            cwd,
            encoding: 'buffer',
        });
        proc.stdout.on('data', function stdout(chunk) {
            if (!decode(chunk).match('OCR init completed.')) return;
            proc.stdout.off('data', stdout);
            return res(proc);
        });
        if (debug) {
            proc.stdout.on('data', (chunk) => {
                console.log(decode(chunk));
            });
            proc.stderr.on('data', (data) => {
                console.log(decode(data));
            });
            proc.on('close', (code) => {
                console.log('close code: ', code);
            });
            proc.on('exit', (code) => {
                console.log('exit code: ', code);
            });
        }
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