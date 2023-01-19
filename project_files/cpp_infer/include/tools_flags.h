// Copyright (c) 2022 hiroi-sora. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// https://github.com/hiroi-sora/PaddleOCR-json

#ifndef _tools_flags_
#define _tools_flags_

// 程序内部使用，不输出的标志码
#define CODE_INIT                0   // 每回合初始值，回合结束时仍为它代表受管控的区域内未发现错误
#define CODE_ERR_MAT_NULL       -999 // OCR读到的mat为空。借助 cls_label 来传递。

// 输出的标志码

// 识别成功
#define CODE_OK                 100 // 成功，且识别出文字
#define CODE_OK_NONE            101 // 成功，且未识别到文字
#define MSG_OK_NONE(p)          "No text found in image. Path: \""+p+"\""
// 按路径读图，失败
#define CODE_ERR_PATH_EXIST     200 // 图片路径不存在
#define MSG_ERR_PATH_EXIST(p)   "Image path dose not exist. Path: \""+p+"\""
#define CODE_ERR_PATH_CONV      201 // 图片路径string无法转换到wstring
#define MSG_ERR_PATH_CONV(p)    "Image path failed to convert to utf-16 wstring. Path: \""+p+"\""
#define CODE_ERR_PATH_READ      202 // 图片路径存在，但无法打开文件
#define MSG_ERR_PATH_READ(p)    "Image open failed. Path: \""+p+"\""
#define CODE_ERR_PATH_DECODE    203 // 图片打开成功，但读取到的内容无法被opencv解码
#define MSG_ERR_PATH_DECODE(p)  "Image decode failed. Path: \""+p+"\""
// 剪贴板读图，失败
#define CODE_ERR_CLIP_OPEN      210 // 剪贴板打开失败 ( OpenClipboard )
#define MSG_ERR_CLIP_OPEN       "Clipboard open failed."
#define CODE_ERR_CLIP_EMPTY     211 // 剪贴板为空 ( GetPriorityClipboardFormat NULL )
#define MSG_ERR_CLIP_EMPTY      "Clipboard is empty."
#define CODE_ERR_CLIP_FORMAT    212 // 剪贴板的格式不支持 ( GetPriorityClipboardFormat -1 )
#define MSG_ERR_CLIP_FORMAT     "Clipboard format is not valid."
#define CODE_ERR_CLIP_DATA      213 // 剪贴板获取内容句柄失败，通常由别的程序占用剪贴板引起 ( GetClipboardData NULL )
#define MSG_ERR_CLIP_DATA       "Getting clipboard data handle failed."
#define CODE_ERR_CLIP_FILES     214 // 剪贴板查询到的文件的数量不为1 ( DragQueryFile != 1 )
#define MSG_ERR_CLIP_FILES(n)   "Clipboard number of query files is not valid. Number: "+std::to_string(n)
#define CODE_ERR_CLIP_GETOBJ    215 // 剪贴板检索图形对象信息失败 ( GetObject NULL )
#define MSG_ERR_CLIP_GETOBJ     "Clipboard get bitmap object failed."
#define CODE_ERR_CLIP_BITMAP    216 // 剪贴板获取位图数据失败 ( GetBitmapBits 复制字节为空 )
#define MSG_ERR_CLIP_BITMAP     "Getting clipboard bitmap bits failed."
#define CODE_ERR_CLIP_CHANNEL   217 // 剪贴板中位图的通道数不支持 ( nChannels 不为1，3，4 )
#define MSG_ERR_CLIP_CHANNEL(n) "Clipboard number of image channels is not valid. Number: "+std::to_string(n)
// 未知
#define CODE_ERR_UNKNOW        299 // 未知异常
#define MSG_ERR_UNKNOW         "An unknown error has occurred."



#endif