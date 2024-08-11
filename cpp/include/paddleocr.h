// Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
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

#pragma once

#include <include/ocr_cls.h>
#include <include/ocr_det.h>
#include <include/ocr_rec.h>

namespace PaddleOCR
{
    class PPOCR
    {
    public:
        explicit PPOCR();
        ~PPOCR() = default; // 默认析构函数

        // OCR方法，处理图像列表，返回每个图像的OCR结果向量
        std::vector<std::vector<OCRPredictResult>> ocr(std::vector<cv::Mat> img_list,
                                                       bool det = true,
                                                       bool rec = true,
                                                       bool cls = true);
        // OCR方法，处理单个图像，返回OCR结果
        std::vector<OCRPredictResult> ocr(cv::Mat img, bool det = true,
                                          bool rec = true, bool cls = true);

        void reset_timer();              // 重置计时器
        void benchmark_log(int img_num); // 记录基准测试日志，参数为图像数量

        // 智能指针
        std::unique_ptr<DBDetector> detector_;       // 指向 文本检测器实例
        std::unique_ptr<Classifier> classifier_;     // 指向 方向分类器实例
        std::unique_ptr<CRNNRecognizer> recognizer_; // 指向 文本识别器实例

    protected:
        // 时间信息
        std::vector<double> time_info_det = {0, 0, 0};
        std::vector<double> time_info_rec = {0, 0, 0};
        std::vector<double> time_info_cls = {0, 0, 0};

        // 文本检测：输入单张图片，在ocr_results向量中存放单行文本碎图的检测信息
        void det(cv::Mat img,
                 std::vector<OCRPredictResult> &ocr_results);
        // 方向分类：输入单行碎图向量，在ocr_results向量中存放每个碎图的方向标志
        void cls(std::vector<cv::Mat> img_list,
                 std::vector<OCRPredictResult> &ocr_results);
        // 文本识别：输入单行碎图向量，在ocr_results向量中存放每个碎图的文本
        void rec(std::vector<cv::Mat> img_list,
                 std::vector<OCRPredictResult> &ocr_results);
    };
} // namespace PaddleOCR
