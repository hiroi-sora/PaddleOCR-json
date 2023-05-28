"use strict";
var __extends = (this && this.__extends) || (function () {
    var extendStatics = function (d, b) {
        extendStatics = Object.setPrototypeOf ||
            ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
            function (d, b) { for (var p in b) if (Object.prototype.hasOwnProperty.call(b, p)) d[p] = b[p]; };
        return extendStatics(d, b);
    };
    return function (d, b) {
        if (typeof b !== "function" && b !== null)
            throw new TypeError("Class extends value " + String(b) + " is not a constructor or null");
        extendStatics(d, b);
        function __() { this.constructor = d; }
        d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
    };
})();
var worker_threads_1 = require("worker_threads");
var path_1 = require("path");
var Queue = /** @class */ (function (_super) {
    __extends(Queue, _super);
    function Queue() {
        return _super !== null && _super.apply(this, arguments) || this;
    }
    Queue.prototype.out = function () {
        var _this = this;
        if (!this.length)
            return;
        new Promise(function (res) { return _this[0](res); })
            .then(function () { return (_super.prototype.shift.call(_this), _this.out()); });
    };
    Queue.prototype.in = function (fn) {
        _super.prototype.push.call(this, fn) - 1 || this.out();
        return true;
    };
    return Queue;
}(Array));
var quqeMap = new WeakMap();
var OCR = /** @class */ (function (_super) {
    __extends(OCR, _super);
    function OCR(path, args, options, debug) {
        var _this = _super.call(this, (0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path: path, args: args, options: options, debug: debug },
            stdout: true,
        }) || this;
        _this.exitCode = null;
        var quqe = new Queue();
        quqeMap.set(_this, quqe);
        quqe.in(function (next) {
            _this.stdout.read();
            _this.stdout.once('data', function (data) {
                var _a = String(data).match(/^pid=(\d+), socket=((\d+\.\d+\.\d+\.\d+):(\d+))?/), pid = _a[1], addr = _a[3], port = _a[4];
                _this.pid = Number(pid);
                _this.addr = addr;
                _this.port = Number(port);
                _super.prototype.emit.call(_this, 'init', _this.pid, _this.addr, _this.port);
                next();
            });
        });
        _super.prototype.once.call(_this, 'exit', function (code) {
            _this.exitCode = code;
            quqeMap.delete(_this);
        });
        return _this;
    }
    OCR.prototype.postMessage = function (obj) {
        var _this = this;
        var _a;
        (_a = quqeMap.get(this)) === null || _a === void 0 ? void 0 : _a.in(function (next) {
            _super.prototype.once.call(_this, 'message', next);
            _super.prototype.postMessage.call(_this, obj);
        });
    };
    OCR.prototype.flush = function (obj) {
        var _this = this;
        return new Promise(function (res) {
            quqeMap.get(_this).in(function (next) {
                _super.prototype.once.call(_this, 'message', function (data) {
                    res(data);
                    next();
                });
                _super.prototype.postMessage.call(_this, obj);
            });
        });
    };
    return OCR;
}(worker_threads_1.Worker));
module.exports = OCR;
