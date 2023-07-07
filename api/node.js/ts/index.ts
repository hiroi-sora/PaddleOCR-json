import { Worker } from 'worker_threads';
import { resolve as path_resolve } from 'path';

interface Queue<T> extends AsyncGenerator<T, null, (resolve: (value?: T) => void, reject: (reason?: any) => void) => void> { }
async function* Queue<T>(value?: T): Queue<T> {
    while (true) value = await new Promise(yield value);
}
const quqeMap = new WeakMap<OCR, Queue<OCR.coutReturnType>>();

class OCR extends Worker {
    pid: number;
    addr: string | undefined;
    port: number | undefined;
    exitCode: number | null = null;
    constructor(path?: string, args?: string[], options?: OCR.Options, debug?: boolean) {
        super(path_resolve(__dirname, 'worker.js'), {
            workerData: { path, args, options, debug },
            stdout: true,
        });
        const quqe = Queue<OCR.coutReturnType>();
        quqeMap.set(this, quqe);
        quqe.next();
        quqe.next((res) => {
            this.stdout.once('data', (data) => {
                const [, pid, socket, addr, port] = String(data).match(/^pid=(\d+)(, a=(\d+\.\d+\.\d+\.\d+:\d+))?/);
                this.pid = Number(pid);
                if (socket) {
                    this.addr = addr;
                    this.port = Number(port);
                }
                super.emit('init', this.pid, this.addr, this.port);
                res();
            });
        });
        super.once('exit', (code) => {
            this.exitCode = code;
            quqeMap.get(this).return(null);
        });
    }
    postMessage(obj: OCR.Arg) { OCR.prototype.flush.call(this, obj); }
    async flush(obj: OCR.Arg) {
        return (await quqeMap.get(this).next((res) => {
            super.once('message', res);
            super.postMessage(obj);
        })).value;
    }
}

namespace OCR {

    interface BaseArg {
        limit_side_len?: number;
        limit_type?: string;
        visualize?: boolean;
        output?: string;
        /* ... */
    }

    interface Arg_Path extends BaseArg {
        image_path?: string | null;
    }
    interface Arg_Base64 extends BaseArg {
        image_base64?: string;
    }

    export type Arg = Arg_Base64 | Arg_Path;

    export interface coutReturnType {
        code: number;
        message: string;
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]],
            score: number,
            text: string,
        }[] | null;
    }

    export type Options = Omit<
        import('child_process').SpawnOptionsWithStdioTuple<'pipe', 'pipe', 'pipe'>,
        keyof typeof import('./worker').__default.options
    >;
}

export = OCR;
