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

// 版本信息
#define PROJECT_VER "v1.3.0"
#define PROJECT_NAME "PaddleOCR-json " PROJECT_VER

#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>

#include <include/args.h>
#include <include/paddleocr.h>
#include <include/paddlestructure.h>
#include <include/task.h>

using namespace PaddleOCR;

void structure(std::vector<cv::String> &cv_all_img_names)
{
    PaddleOCR::PaddleStructure engine = PaddleOCR::PaddleStructure();

    if (FLAGS_benchmark)
    {
        engine.reset_timer();
    }

    for (int i = 0; i < cv_all_img_names.size(); i++)
    {
        std::cout << "predict img: " << cv_all_img_names[i] << std::endl;
        cv::Mat img = cv::imread(cv_all_img_names[i], cv::IMREAD_COLOR);
        if (!img.data)
        {
            std::cerr << "[ERROR] image read failed! image path: "
                      << cv_all_img_names[i] << std::endl;
            continue;
        }

        std::vector<StructurePredictResult> structure_results = engine.structure(
            img, FLAGS_layout, FLAGS_table, FLAGS_det && FLAGS_rec);

        for (int j = 0; j < structure_results.size(); j++)
        {
            std::cout << j << "\ttype: " << structure_results[j].type
                      << ", region: [";
            std::cout << structure_results[j].box[0] << ","
                      << structure_results[j].box[1] << ","
                      << structure_results[j].box[2] << ","
                      << structure_results[j].box[3] << "], score: ";
            std::cout << structure_results[j].confidence << ", res: ";

            if (structure_results[j].type == "table")
            {
                std::cout << structure_results[j].html << std::endl;
                if (structure_results[j].cell_box.size() > 0 && FLAGS_visualize)
                {
                    std::string file_name = Utility::basename(cv_all_img_names[i]);

                    Utility::VisualizeBboxes(img, structure_results[j],
                                             FLAGS_output + "/" + std::to_string(j) +
                                                 "_" + file_name);
                }
            }
            else
            {
                std::cout << "count of ocr result is : "
                          << structure_results[j].text_res.size() << std::endl;
                if (structure_results[j].text_res.size() > 0)
                {
                    std::cout << "********** print ocr result "
                              << "**********" << std::endl;
                    Utility::print_result(structure_results[j].text_res);
                    std::cout << "********** end print ocr result "
                              << "**********" << std::endl;
                }
            }
        }
    }
    if (FLAGS_benchmark)
    {
        engine.benchmark_log(cv_all_img_names.size());
    }
}

int main(int argc, char **argv)
{
    std::cout << PROJECT_NAME << std::endl; // 版本提示
    // 读取命令行
    google::ParseCommandLineFlags(&argc, &argv, true);
    // 读取配置文件
    std::string configMsg = read_config();
    if (!configMsg.empty())
    {
        std::cerr << configMsg << std::endl;
    }
    // 检查参数合法性
    std::string checkMsg = check_flags();
    if (!checkMsg.empty())
    {
        std::cerr << "[ERROR] " << checkMsg << std::endl;
        return 1;
    }

    // 启动任务
    Task task = Task();
    if (FLAGS_type == "ocr")
    { // OCR图片模式
        return task.ocr();
    }
    // TODO: 图表识别模式
    else if (FLAGS_type == "structure")
    {
        std::cerr << "[ERROR] structure not support. " << std::endl;
        // structure(cv_all_img_names);
    }
}
