"use strict";
var __assign = (this && this.__assign) || function () {
    __assign = Object.assign || function(t) {
        for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p))
                t[p] = s[p];
        }
        return t;
    };
    return __assign.apply(this, arguments);
};
Object.defineProperty(exports, "__esModule", { value: true });
var worker_threads_1 = require("worker_threads");
var path_1 = require("path");
var net_1 = require("net");
var child_process_1 = require("child_process");
var __default = {
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
    var currentPath = process.cwd();
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
var end = 'at' in String ? function (str) { return str.at(-1); } : function (str) { return str[str.length - 1]; };
if (!worker_threads_1.isMainThread) {
    var _a = worker_threads_1.workerData, _b = _a.path, path = _b === void 0 ? __default.path : _b, _c = _a.args, args = _c === void 0 ? [] : _c, options = _a.options, debug_1 = _a.debug;
    var mode_1 = 0;
    var proc_1 = (0, child_process_1.spawn)(path, args.concat(__default.args), __assign(__assign({}, options), __default.options));
    process.once('exit', proc_1.kill.bind(proc_1));
    proc_1.once('exit', process.exit);
    new Promise(function (res) { return proc_1.stdout.on('data', function stdout(chunk) {
        var data = chunk.toString();
        if (!mode_1) {
            data.match(__default.pipeTag) && (mode_1 = 1);
            data.match(__default.socketTag) && (mode_1 = 2);
        }
        if (!data.match(__default.initTag))
            return;
        proc_1.stdout.off('data', stdout);
        return res();
    }); }).then(function () { return new Promise(function (res) {
        if (mode_1 === 1) {
            process.stdout.write("pid=".concat(proc_1.pid, ", pipe=true\n"));
            return res();
        }
        proc_1.stderr.once('data', function () { return null; });
        proc_1.stdout.once('data', function (chunk) {
            var data = chunk.toString();
            var socket = data.match(__default.socketMatch)[1].split(':');
            process.stdout.write("pid=".concat(proc_1.pid, ", addr=").concat(socket[0], ", port=").concat(socket[1], "\n"));
            return res([socket[0], Number(socket[1])]);
        });
    }); }).then(function (socket) {
        if (debug_1) {
            proc_1.stdout.pipe(process.stdout);
            proc_1.stderr.pipe(process.stderr);
        }
        else if (socket) {
            proc_1.stdout.destroy();
            proc_1.stderr.destroy();
        }
        if (socket) {
            var client_1 = new net_1.Socket();
            var addr_1 = socket[0], port_1 = socket[1];
            worker_threads_1.parentPort.on('message', function (data) {
                client_1.connect(port_1, addr_1, function () {
                    client_1.end("".concat(JSON.stringify(cargs(data)), "\n"));
                });
            });
            client_1.on('data', function (chunk) {
                worker_threads_1.parentPort.postMessage(cout(JSON.parse(String(chunk))));
            });
        }
        else {
            worker_threads_1.parentPort.on('message', function (data) {
                proc_1.stdin.write("".concat(JSON.stringify(cargs(data)), "\n"));
            });
            var cache_1 = [];
            proc_1.stdout.on('data', function (chunk) {
                var str = String(chunk);
                cache_1.push(str);
                if (end(str) !== '\n')
                    return;
                worker_threads_1.parentPort.postMessage(cout(JSON.parse(cache_1.join(''))));
                cache_1.length = 0;
            });
        }
    });
}
