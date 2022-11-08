import { isMainThread, parentPort, workerData } from 'worker_threads';
import { resolve as path_resolve } from 'path';
import { spawn, SpawnOptionsWithStdioTuple, StdioPipe, ChildProcessWithoutNullStreams } from 'child_process';

const currentPath = process.cwd();

export interface DArg {
    image_dir?: string | null;
    limit_side_len?: number;
    limit_type?: string;
    visualize?: boolean;
    output?: string;
}

const pipe: StdioPipe = 'pipe';
export const __default = {
    path: 'PaddleOCR_json.exe',
    args: ['--use_debug=0'],
    options: {
        argv0: undefined,
        stdio: pipe,
        detached: false,
        shell: false,
        windowsVerbatimArguments: undefined,
        windowsHide: true,
    },
    initTag: 'OCR init completed.',
}

export type Options = Omit<SpawnOptionsWithStdioTuple<StdioPipe, StdioPipe, StdioPipe>, keyof typeof __default.options>;

function cargs(obj: DArg) {
    obj = Object.assign({}, obj);
    if (obj.image_dir === null) obj.image_dir = 'clipboard';
    else if (obj.image_dir) obj.image_dir = path_resolve(currentPath, obj.image_dir);
    if (obj.output !== undefined)
        obj.output = path_resolve(currentPath, obj.output);
    return obj;
}
function cout(data: { code: number, data: any }): {
    code: number;
    message: string;
    pid?: number;
    data: {
        box: [[number, number], [number, number], [number, number], [number, number]],
        score: number,
        text: string,
    }[] | null;
} {
    return {
        code: data.code,
        message: data.code - 100 ? data.data : '',
        data: data.code - 100 ? null : data.data,
    };
}
export type coutReturnType = ReturnType<typeof cout>;

if (!isMainThread) {
    new Promise((res: (v: ChildProcessWithoutNullStreams) => void) => {
        const {
            path = __default.path,
            args = [],
            options,
            debug = false,
        }: {
            path: string,
            args?: string[],
            options?: Options,
            debug: boolean
        } = workerData;
        const proc = spawn(path, args.concat(__default.args), Object.assign({}, options, __default.options));
        proc.stdout.on('data', function stdout(chunk) {
            if (!chunk.toString().match(__default.initTag)) return;
            proc.stdout.off('data', stdout);
            return res(proc);
        });
        if (debug) {
            proc.stdout.on('data', (chunk) => {
                console.log(chunk.toString());
            });
            proc.stderr.on('data', (data) => {
                console.log(data.toString());
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
            message: __default.initTag,
            pid: proc.pid,
            data: null,
        });
        parentPort.on('message', (data) => {
            proc.stdin.write(`${JSON.stringify(cargs(data))}\n`)
        });
        proc.stdout.on('data', (chunk) => {
            parentPort.postMessage(cout(JSON.parse(chunk)));
        });
    });
}