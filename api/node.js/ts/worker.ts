import { isMainThread, parentPort, workerData } from 'worker_threads';
import { resolve as path_resolve } from 'path';
import { spawn, type ChildProcessWithoutNullStreams } from 'child_process';
import type { DArg, coutReturnType, Options } from './index';

interface workerData {
    path: string;
    args?: string[];
    options?: Options;
    debug: boolean;
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

if (!isMainThread) {
    new Promise((res: (v: ChildProcessWithoutNullStreams) => void) => {

        const {
            path = __default.path,
            args = [],
            options,
            debug = false,
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

        if (!debug) return;
        proc.stdout.on('data', (chunk) =>
            console.log(chunk.toString())
        );
        proc.stderr.on('data', (data) =>
            console.log(data.toString())
        );
        proc.on('close', (code) =>
            console.log('close code: ', code)
        ).on('exit', (code) =>
            console.log('exit code: ', code)
        );
    }).then((proc) => {
        process.stdout.end(String(proc.pid));
        parentPort.on('message', (data) => {
            proc.stdin.write(`${JSON.stringify(cargs(data))}\n`);
        });
        const cache = [];
        proc.stdout.on('data', (chunk) => {
            const str = String(chunk);
            cache.push(str);
            if (str[str.length - 1] !== '\n') return;
            parentPort.postMessage(cout(JSON.parse(cache.join('')))); cache.length = 0;
        });
    });
}
