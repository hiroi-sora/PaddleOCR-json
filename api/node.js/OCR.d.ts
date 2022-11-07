import { Worker } from 'worker_threads';
import { SpawnOptionsWithStdioTuple, StdioPipe } from 'child_process';
interface DArg {
    image_dir?: string | null;
    limit_side_len?: number;
    limit_type?: string;
    visualize?: boolean;
    output?: string;
}
declare class OCR extends Worker {
    constructor(path?: string, args?: string[], options?: Omit<SpawnOptionsWithStdioTuple<StdioPipe, StdioPipe, StdioPipe>, 'argv0' | 'stdio' | 'detached' | 'shell' | 'windowsVerbatimArguments' | 'windowsHide'>, debug?: boolean);
    get length(): number;
    postMessage(obj: DArg): Promise<{
        code: number;
        message: string;
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]];
            score: number;
            text: string;
        }[] | null;
    }>;
    flush(obj: DArg): Promise<{
        code: number;
        message: string;
        data: {
            box: [[number, number], [number, number], [number, number], [number, number]];
            score: number;
            text: string;
        }[] | null;
    }>;
}
export = OCR;
