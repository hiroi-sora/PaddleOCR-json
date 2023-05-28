"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const worker_threads_1 = require("worker_threads");
const path_1 = require("path");
const net_1 = require("net");
const child_process_1 = require("child_process");
const currentPath = process.cwd();
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
    initTag: /^OCR init completed\./,
    socketTag: /^OCR socket mode\./,
    socketMatch: /^Socket init completed\. (\d+\.\d+\.\d+\.\d+:\d+)/,
};
function cargs(obj) {
    obj = Object.assign({}, obj);
    if (obj.image_path === null)
        obj.image_path = 'clipboard';
    else if (obj.image_path)
        obj.image_path = (0, path_1.resolve)(currentPath, obj.image_path);
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
    let socket;
    const proc = (0, child_process_1.spawn)(path, args.concat(__default.args), {
        ...options,
        ...__default.options,
    });
    process.once('exit', proc.kill.bind(proc));
    proc.once('exit', process.exit);
    new Promise((res) => {
        let socketMode = false;
        proc.stdout.on('data', function stdout(chunk) {
            const data = chunk.toString();
            if (!socketMode && data.match(__default.socketTag))
                socketMode = true;
            if (socketMode) {
                socket = data.match(__default.socketMatch)?.[1];
                if (!socket)
                    return;
            }
            else {
                if (!data.match(__default.initTag))
                    return;
            }
            proc.stdout.off('data', stdout), res();
        });
    }).then(() => {
        process.stdout.write(`pid=${proc.pid}, socket=${socket ?? ''}\n`);
        process.stdout.clearLine;
        if (debug) {
            proc.stdout.pipe(process.stdout);
            proc.stderr.pipe(process.stderr);
        }
        else {
            process.stdout.end();
            proc.stdout.destroy();
            proc.stderr.destroy();
        }
        if (socket) {
            const client = new net_1.Socket();
            const [addr, port] = ((socket) => [socket[0], Number(socket[1])])(socket.split(':'));
            worker_threads_1.parentPort.on('message', (data) => {
                client.connect(port, addr, () => {
                    client.end(JSON.stringify(cargs(data)));
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
