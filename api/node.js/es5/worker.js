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
var child_process_1 = require("child_process");
var currentPath = process.cwd();
var __default = {
    path: 'PaddleOCR_json.exe',
    args: ['--use_debug=0'],
    options: {
        argv0: undefined,
        stdio: 'pipe',
        detached: false,
        shell: false,
        windowsVerbatimArguments: undefined,
        windowsHide: true,
    },
    initTag: 'OCR init completed.',
};
function cargs(obj) {
    obj = Object.assign({}, obj);
    if (obj.image_dir === null)
        obj.image_dir = 'clipboard';
    else if (obj.image_dir)
        obj.image_dir = (0, path_1.resolve)(currentPath, obj.image_dir);
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
if (!worker_threads_1.isMainThread) {
    new Promise(function (res) {
        var _a = worker_threads_1.workerData, _b = _a.path, path = _b === void 0 ? __default.path : _b, _c = _a.args, args = _c === void 0 ? [] : _c, options = _a.options, _d = _a.debug, debug = _d === void 0 ? false : _d;
        var proc = (0, child_process_1.spawn)(path, args.concat(__default.args), __assign(__assign({}, options), __default.options));
        proc.stdout.on('data', function stdout(chunk) {
            if (!chunk.toString().match(__default.initTag))
                return;
            proc.stdout.off('data', stdout);
            return res(proc);
        }).on('exit', process.exit);
        if (!debug)
            return;
        proc.stdout.on('data', function (chunk) {
            return console.log(chunk.toString());
        });
        proc.stderr.on('data', function (data) {
            return console.log(data.toString());
        });
        proc.on('close', function (code) {
            return console.log('close code: ', code);
        }).on('exit', function (code) {
            return console.log('exit code: ', code);
        });
    }).then(function (proc) {
        process.stdout.end(String(proc.pid));
        worker_threads_1.parentPort.on('message', function (data) {
            proc.stdin.write("".concat(JSON.stringify(cargs(data)), "\n"));
        });
        var cache = [];
        proc.stdout.on('data', function (chunk) {
            var str = String(chunk);
            cache.push(str);
            if (str[str.length - 1] !== '\n')
                return;
            worker_threads_1.parentPort.postMessage(cout(JSON.parse(cache.join(''))));
            cache.length = 0;
        });
    });
}
