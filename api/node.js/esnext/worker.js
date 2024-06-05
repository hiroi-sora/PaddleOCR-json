"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const worker_threads_1 = require("worker_threads");
const path_1 = require("path");
const net_1 = require("net");
const child_process_1 = require("child_process");
const __default = {
    path: 'PaddleOCR-json.exe',
    args: [],
    options: {
        argv0: undefined,
        stdio: 'pipe',
        detached: false,
        shell: false,
        windowsVerbatimArguments: undefined,
        windowsHide: true,
    },
    initTag: 'OCR init completed.',
    socketTag: 'OCR socket mode.',
    pipeTag: 'OCR anonymous pipe mode.',
    socketMatch: /^Socket init completed. (\d+\.\d+\.\d+\.\d+:\d+)/,
};
function cargs(obj) {
    const currentPath = process.cwd();
    obj = Object.assign({}, obj);
    if ('image_path' in obj) {
        if (obj.image_path === null)
            throw new Error('图片路径为空。');
        else if (obj.image_path)
            obj.image_path = (0, path_1.resolve)(currentPath, obj.image_path);
    }
    if (obj.output !== undefined)
        obj.output = (0, path_1.resolve)(currentPath, obj.output);
    return obj;
}
function cout(data) {
    return {
        code: data.code,
        message: data.code - 100 ? data.data : '',
        data: data.code - 100 ? null : data.data,
    };
}
const end = 'at' in String ? (str) => str.at(-1) : (str) => str[str.length - 1];
if (!worker_threads_1.isMainThread) {
    const { path = __default.path, args = [], options, debug, } = worker_threads_1.workerData;
    let mode = 0;
    const proc = (0, child_process_1.spawn)(path, args.concat(__default.args), {
        ...options,
        ...__default.options,
    });
    process.once('exit', proc.kill.bind(proc));
    proc.once('exit', process.exit);
    new Promise((res) => proc.stdout.on('data', function stdout(chunk) {
        const data = chunk.toString();
        if (!mode) {
            data.match(__default.pipeTag) && (mode = 1);
            data.match(__default.socketTag) && (mode = 2);
        }
        if (!data.match(__default.initTag))
            return;
        proc.stdout.off('data', stdout);
        return res();
    })).then(() => new Promise((res) => {
        if (mode === 1) {
            process.stdout.write(`pid=${proc.pid}, pipe=true\n`);
            return res();
        }
        proc.stderr.once('data', () => null);
        proc.stdout.once('data', (chunk) => {
            const data = chunk.toString();
            const socket = data.match(__default.socketMatch)[1].split(':');
            process.stdout.write(`pid=${proc.pid}, addr=${socket[0]}, port=${socket[1]}\n`);
            return res([socket[0], Number(socket[1])]);
        });
    })).then((socket) => {
        if (debug) {
            proc.stdout.pipe(process.stdout);
            proc.stderr.pipe(process.stderr);
        }
        else if (socket) {
            proc.stdout.destroy();
            proc.stderr.destroy();
        }
        if (socket) {
            const client = new net_1.Socket();
            const [addr, port] = socket;
            worker_threads_1.parentPort.on('message', (data) => {
                client.connect(port, addr, () => {
                    client.end(`${JSON.stringify(cargs(data))}\n`);
                });
            });
            client.on('data', (chunk) => {
                worker_threads_1.parentPort.postMessage(cout(JSON.parse(String(chunk))));
            });
        }
        else {
            worker_threads_1.parentPort.on('message', (data) => {
                proc.stdin.write(`${JSON.stringify(cargs(data))}\n`);
            });
            const cache = [];
            proc.stdout.on('data', (chunk) => {
                const str = String(chunk);
                cache.push(str);
                if (end(str) !== '\n')
                    return;
                worker_threads_1.parentPort.postMessage(cout(JSON.parse(cache.join(''))));
                cache.length = 0;
            });
        }
    });
}
