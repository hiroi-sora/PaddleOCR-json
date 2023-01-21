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
        Queue.prototype.shift = function () {
            var _this = this;
            if (this.length)
                new Promise(function (res) { return _this[0](res); }).then(function () { return (_super.prototype.shift.call(_this), _this.shift()); });
        };
        Queue.prototype.push = function (fn) {
            _super.prototype.push.call(this, fn) - 1 || this.shift();
        };
        return Queue;
    }(Array));
    var map = new /** @class */ (function (_super) {
        __extends(class_1, _super);
        function class_1() {
            return _super !== null && _super.apply(this, arguments) || this;
        }
        class_1.prototype.get = function (key) {
            var _this = this;
            return _super.prototype.get.call(this, key) || (function (quqe) { return (_super.prototype.set.call(_this, key, quqe), quqe); })(new Queue());
        };
        return class_1;
    }(WeakMap))();
    return map.get.bind(map);
})();
var OCR = /** @class */ (function (_super) {
    __extends(OCR, _super);
    function OCR(path, args, options, debug) {
        if (debug === void 0) { debug = false; }
        var _this = _super.call(this, (0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path: path, args: args, options: options, debug: debug },
        }) || this;
        $quqe(_this).push(function (next) {
            return _super.prototype.once.call(_this, 'message', function (data) { return (_super.prototype.emit.call(_this, 'init', _this.pid = data.pid), next()); });
        });
        return _this;
    }
    OCR.prototype.emit = function () {
        var args = [];
        for (var _i = 0; _i < arguments.length; _i++) {
            args[_i] = arguments[_i];
        }
        if (args[0] === 'init')
            return false;
        return _super.prototype.emit.apply(this, args);
    };
    OCR.prototype.postMessage = function (obj) {
        var _this = this;
        $quqe(this).push(function (next) {
            _super.prototype.once.call(_this, 'message', next);
            _super.prototype.postMessage.call(_this, obj);
        });
    };
    OCR.prototype.flush = function (obj) {
        var _this = this;
        return new Promise(function (res) {
            return $quqe(_this).push(function (next) {
                _super.prototype.once.call(_this, 'message', function (data) { return (res(data), next()); });
                _super.prototype.postMessage.call(_this, obj);
            });
        });
    };
    return OCR;
}(worker_threads_1.Worker));
module.exports = OCR;
