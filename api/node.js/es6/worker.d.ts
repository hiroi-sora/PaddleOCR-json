/// <reference types="node" />
import { SpawnOptionsWithStdioTuple, StdioPipe } from 'child_process';
export interface DArg {
    image_dir?: string | null;
    limit_side_len?: number;
    limit_type?: string;
    visualize?: boolean;
    output?: string;
}
export declare const __default: {
    path: string;
    args: string[];
    options: {
        argv0: any;
        stdio: "pipe";
        detached: boolean;
        shell: boolean;
        windowsVerbatimArguments: any;
        windowsHide: boolean;
    };
    initTag: string;
};
export declare type Options = Omit<SpawnOptionsWithStdioTuple<StdioPipe, StdioPipe, StdioPipe>, keyof typeof __default.options>;
declare function cout(data: {
    code: number;
    data: any;
}): {
    code: number;
    message: string;
    pid?: number;
    data: {
        box: [[number, number], [number, number], [number, number], [number, number]];
        score: number;
        text: string;
    }[] | null;
};
export declare type coutReturnType = ReturnType<typeof cout>;
export {};
