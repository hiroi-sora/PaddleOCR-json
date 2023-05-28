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
var $quqe = (function () {
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
        };
        return Queue;
    }(Array));
    var map = new WeakMap();
    return function (key) {
        var value = map.get(key);
        if (value)
            return value;
        value = new Queue();
        map.set(key, value);
        return value;
    };
})();
var OCR = /** @class */ (function (_super) {
    __extends(OCR, _super);
    function OCR(path, args, options) {
        var _this = _super.call(this, (0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path: path, args: args, options: options },
            stdout: true,
        }) || this;
        $quqe(_this).in(function (next) {
            _this.stdout.read();
            _this.stdout.once('data', function (pid) {
                _super.prototype.emit.call(_this, 'init', _this.pid = Number(String(pid)));
                next();
            });
        });
        return _this;
    }
    OCR.prototype.postMessage = function (obj) {
        var _this = this;
        $quqe(this).in(function (next) {
            _super.prototype.once.call(_this, 'message', next);
            _super.prototype.postMessage.call(_this, obj);
        });
    };
    OCR.prototype.flush = function (obj) {
        var _this = this;
        return new Promise(function (res) {
            return $quqe(_this).in(function (next) {
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
