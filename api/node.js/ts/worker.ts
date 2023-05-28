import { isMainThread, parentPort, workerData } from 'worker_threads';
import { resolve as path_resolve } from 'path';
import { spawn, type ChildProcessWithoutNullStreams } from 'child_process';
import type { DArg, coutReturnType, Options } from './index';

interface workerData {
    path: string;
    args?: string[];
    options?: Options;
}

const currentPath = process.cwd();

const __default = {
    path: 'PaddleOCR_json.exe',
    args: ['--use_debug=0'],
    options: {
        argv0: undefined,
        stdio: 'pipe' as const,
        detached: false,
        shell: false,
        windowsVerbatimArguments: undefined,
        windowsHide: true,
    },
    initTag: 'OCR init completed.',
};
export { type __default };

function cargs(obj: DArg) {
    obj = Object.assign({}, obj);
    if (obj.image_dir === null) obj.image_dir = 'clipboard';
    else if (obj.image_dir) obj.image_dir = path_resolve(currentPath, obj.image_dir);
    if (obj.output !== undefined)
        obj.output = path_resolve(currentPath, obj.output);
    return obj;
}
function cout(data: { code: number, data: any; }) {
    return {
        code: data.code,
        message: data.code - 100 ? data.data : '',
        data: data.code - 100 ? null : data.data,
    } as coutReturnType;
}

const end = 'at' in String ? (str: string) => str.at(-1) : (str: string) => str[str.length - 1];

if (!isMainThread) {
    new Promise((res: (v: ChildProcessWithoutNullStreams) => void) => {

        const {
            path = __default.path,
            args = [],
            options,
        } = workerData as workerData;

        const proc = spawn(path, args.concat(__default.args), {
            ...options,
            ...__default.options,
        });
        process.once('exit', proc.kill.bind(proc));
        proc.once('exit', process.exit);

        proc.stdout.on('data', function stdout(chunk) {
            if (!chunk.toString().match(__default.initTag)) return;
            proc.stdout.off('data', stdout);
            return res(proc);
        });
    }).then((proc) => {
        process.stdout.write(`${proc.pid}\n`);
        process.stdout.clearLine
        proc.stdout.pipe(process.stdout);
        proc.stderr.pipe(process.stderr);
        parentPort.on('message', (data) => {
            proc.stdin.write(`${JSON.stringify(cargs(data))}\n`);
        });
        const cache = [];
        proc.stdout.on('data', (chunk) => {
            const str = String(chunk);
            cache.push(str);
            if (end(str) !== '\n') return;
            parentPort.postMessage(cout(JSON.parse(cache.join('')))); cache.length = 0;
        });
    });
}
