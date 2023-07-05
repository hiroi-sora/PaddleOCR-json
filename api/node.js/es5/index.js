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
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (g && (g = 0, op[0] && (_ = 0)), _) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
var __await = (this && this.__await) || function (v) { return this instanceof __await ? (this.v = v, this) : new __await(v); }
var __asyncGenerator = (this && this.__asyncGenerator) || function (thisArg, _arguments, generator) {
    if (!Symbol.asyncIterator) throw new TypeError("Symbol.asyncIterator is not defined.");
    var g = generator.apply(thisArg, _arguments || []), i, q = [];
    return i = {}, verb("next"), verb("throw"), verb("return"), i[Symbol.asyncIterator] = function () { return this; }, i;
    function verb(n) { if (g[n]) i[n] = function (v) { return new Promise(function (a, b) { q.push([n, v, a, b]) > 1 || resume(n, v); }); }; }
    function resume(n, v) { try { step(g[n](v)); } catch (e) { settle(q[0][3], e); } }
    function step(r) { r.value instanceof __await ? Promise.resolve(r.value.v).then(fulfill, reject) : settle(q[0][2], r); }
    function fulfill(value) { resume("next", value); }
    function reject(value) { resume("throw", value); }
    function settle(f, v) { if (f(v), q.shift(), q.length) resume(q[0][0], q[0][1]); }
};
var worker_threads_1 = require("worker_threads");
var path_1 = require("path");
function Queue(value) {
    return __asyncGenerator(this, arguments, function Queue_1() {
        var _a;
        return __generator(this, function (_b) {
            switch (_b.label) {
                case 0:
                    if (!true) return [3 /*break*/, 4];
                    _a = Promise.bind;
                    return [4 /*yield*/, __await(value)];
                case 1: return [4 /*yield*/, _b.sent()];
                case 2: return [4 /*yield*/, __await.apply(void 0, [new (_a.apply(Promise, [void 0, _b.sent()]))()])];
                case 3:
                    value = _b.sent();
                    return [3 /*break*/, 0];
                case 4: return [2 /*return*/];
            }
        });
    });
}
var quqeMap = new WeakMap();
var OCR = /** @class */ (function (_super) {
    __extends(OCR, _super);
    function OCR(path, args, options, debug) {
        var _this = _super.call(this, (0, path_1.resolve)(__dirname, 'worker.js'), {
            workerData: { path: path, args: args, options: options, debug: debug },
            stdout: true,
        }) || this;
        _this.exitCode = null;
        var quqe = Queue();
        quqeMap.set(_this, quqe);
        quqe.next();
        quqe.next(function (res) {
            _this.stdout.once('data', function (data) {
                var _a = String(data).match(/^pid=(\d+)(, a=(\d+\.\d+\.\d+\.\d+:\d+))?/), pid = _a[1], socket = _a[2], addr = _a[3], port = _a[4];
                _this.pid = Number(pid);
                if (socket) {
                    _this.addr = addr;
                    _this.port = Number(port);
                }
                _super.prototype.emit.call(_this, 'init', _this.pid, _this.addr, _this.port);
                res();
            });
        });
        _super.prototype.once.call(_this, 'exit', function (code) {
            _this.exitCode = code;
            quqeMap.get(_this).return(null);
        });
        return _this;
    }
    OCR.prototype.postMessage = function (obj) { OCR.prototype.flush.call(this, obj); };
    OCR.prototype.flush = function (obj) {
        return __awaiter(this, void 0, void 0, function () {
            var _this = this;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, quqeMap.get(this).next(function (res) {
                            _super.prototype.once.call(_this, 'message', res);
                            _super.prototype.postMessage.call(_this, obj);
                        })];
                    case 1: return [2 /*return*/, (_a.sent()).value];
                }
            });
        });
    };
    return OCR;
}(worker_threads_1.Worker));
module.exports = OCR;
