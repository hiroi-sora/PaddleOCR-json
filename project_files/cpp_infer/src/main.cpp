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
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>

#include <include/args.h>
#include <include/paddleocr.h>
#include <include/paddlestructure.h>

#include <include/load_config.h>

#include "include/nlohmann_json.hpp"
using namespace nlohmann;

using namespace PaddleOCR;

void check_params() {
  if (FLAGS_det) {
    if (FLAGS_det_model_dir.empty()) {
      std::cout << "Usage[det]: ./ppocr "
                   "--det_model_dir=/PATH/TO/DET_INFERENCE_MODEL/ "
                << std::endl;
      system("pause");
      exit(1);
    }
  }
  if (FLAGS_rec) {
    //std::cout
    //    << "In PP-OCRv3, rec_image_shape parameter defaults to '3, 48, 320',"
    //       "if you are using recognition model with PP-OCRv2 or an older "
    //       "version, "
    //       "please set --rec_image_shape='3,32,320"
    //    << std::endl;
    if (FLAGS_rec_model_dir.empty()) {
      std::cout << "Usage[rec]: ./ppocr "
                   "--rec_model_dir=/PATH/TO/REC_INFERENCE_MODEL/ "
                << std::endl;
      system("pause");
      exit(1);
    }
  }
  if (FLAGS_cls && FLAGS_use_angle_cls) {
    if (FLAGS_cls_model_dir.empty()) {
      std::cout << "Usage[cls]: ./ppocr "
                << "--cls_model_dir=/PATH/TO/REC_INFERENCE_MODEL/ "
                << std::endl;
      system("pause");
      exit(1);
    }
  }
  if (FLAGS_table) {
    if (FLAGS_table_model_dir.empty() || FLAGS_det_model_dir.empty() ||
        FLAGS_rec_model_dir.empty()) {
      std::cout << "Usage[table]: ./ppocr "
                << "--det_model_dir=/PATH/TO/DET_INFERENCE_MODEL/ "
                << "--rec_model_dir=/PATH/TO/REC_INFERENCE_MODEL/ "
                << "--table_model_dir=/PATH/TO/TABLE_INFERENCE_MODEL/ "
                << std::endl;
      system("pause");
      exit(1);
    }
  }
  if (FLAGS_precision != "fp32" && FLAGS_precision != "fp16" &&
      FLAGS_precision != "int8") {
    cout << "precison should be 'fp32'(default), 'fp16' or 'int8'. " << endl;
    system("pause");
    exit(1);
  }
}

void ocr(std::vector<cv::String> &cv_all_img_names) {

  PPOCR ocr = PPOCR(); // 初始化识别器 

  std::vector<std::vector<OCRPredictResult>> ocr_results = // 执行一次
      ocr.ocr(cv_all_img_names, FLAGS_det, FLAGS_rec, FLAGS_cls);

  for (int i = 0; i < cv_all_img_names.size(); ++i) {
    if (FLAGS_benchmark) {
      cout << cv_all_img_names[i] << '\t';
      if (FLAGS_rec && FLAGS_det) {
        Utility::print_result(ocr_results[i]);
      } else if (FLAGS_det) {
        for (int n = 0; n < ocr_results[i].size(); n++) {
          for (int m = 0; m < ocr_results[i][n].box.size(); m++) {
            cout << ocr_results[i][n].box[m][0] << ' '
                 << ocr_results[i][n].box[m][1] << ' ';
          }
        }
        cout << endl;
      } else {
        Utility::print_result(ocr_results[i]);
      }
    } else {
      cout << cv_all_img_names[i] << "\n";
      Utility::print_result(ocr_results[i]);
      if (FLAGS_visualize && FLAGS_det) {
        cv::Mat srcimg = cv::imread(cv_all_img_names[i], cv::IMREAD_COLOR);
        if (!srcimg.data) {
          std::cerr << "[ERROR] image read failed! image path: "
                    << cv_all_img_names[i] << endl;
          system("pause");
          exit(1);
        }
        std::string file_name = Utility::basename(cv_all_img_names[i]);

        Utility::VisualizeBboxes(srcimg, ocr_results[i],
                                 FLAGS_output + "/" + file_name);
      }
      cout << "***************************" << endl;
    }
  }
}

void structure(std::vector<cv::String> &cv_all_img_names) {
  PaddleOCR::PaddleStructure engine = PaddleOCR::PaddleStructure();
  std::vector<std::vector<StructurePredictResult>> structure_results =
      engine.structure(cv_all_img_names, false, FLAGS_table);
  for (int i = 0; i < cv_all_img_names.size(); i++) {
    cout << "predict img: " << cv_all_img_names[i] << endl;
    for (int j = 0; j < structure_results[i].size(); j++) {
      std::cout << j << "\ttype: " << structure_results[i][j].type
                << ", region: [";
      std::cout << structure_results[i][j].box[0] << ","
                << structure_results[i][j].box[1] << ","
                << structure_results[i][j].box[2] << ","
                << structure_results[i][j].box[3] << "], res: ";
      if (structure_results[i][j].type == "table") {
        std::cout << structure_results[i][j].html << std::endl;
      } else {
        Utility::print_result(structure_results[i][j].text_res);
      }
    }
  }
}

void runOCR(PPOCR &ocr, string img_path) {
  // 执行一次识别，FLAGS_image_dir只能是单个文件路径。
  if (!Utility::PathExists(img_path)) {
    cout << R"({"code":200,"data":"Image path not exist. Path:\")"<< img_path << R"(\""})" << endl;
    return;
  }

  std::vector<cv::String> cv_all_img_names({ img_path }); // 一次处理一个文件 


  std::vector<std::vector<OCRPredictResult>> ocr_results = // 执行，获取结果
    ocr.ocr(cv_all_img_names, FLAGS_det, FLAGS_rec, FLAGS_cls);


  std::vector<OCRPredictResult> ocr_result = ocr_results[0];

  if (ocr_result.empty()) { // 图中无文字
    cout << R"({"code":101,"data":"No text found in image. Path:\")" << img_path << R"(\""})" << endl;
    return;
  }
  else if (ocr_result[0].cls_label == -201) { // 无法读取图片
    cout << R"({"code":201,"data":"Image read failed. Path:\")" << img_path << R"(\""})" << endl;
    return;
  }

  json outJ;
  outJ["code"] = 100;
  outJ["data"] = json::array();
  bool isEmpty = true;
  for (int i = 0; i < ocr_result.size(); i++) {
    json j;
    j["text"] = ocr_result[i].text;
    j["score"] = ocr_result[i].score;
    std::vector<std::vector<int>> b = ocr_result[i].box;
    if (b.empty() || j["text"]=="" ) // 没有文字，跳过本组 
      continue;
    else
      j["box"] = { b[0][0], b[0][1], b[1][0], b[1][1], 
                   b[2][0], b[2][1], b[3][0], b[3][1], };
    outJ["data"].push_back(j);
    isEmpty = false;
  }
  if(isEmpty)
    cout << R"({"code":101,"data":"No text found in image."})" << endl;
  else {
    FLAGS_ensure_ascii ? 
      cout << outJ.dump(-1,' ',true) << endl // 所有非ascii字符转义为ascii，规避中文编码问题
      : cout << outJ << endl;

    if (FLAGS_visualize && FLAGS_det) { // 结果可视化，用于调试 
      cv::Mat srcimg = cv::imread(img_path, cv::IMREAD_COLOR);
      if (!srcimg.data) {
        std::cerr << "[ERROR] image read failed! image path: "
          << img_path << endl;
        return;
      }
      std::string file_name = Utility::basename(img_path);

      Utility::VisualizeBboxes(srcimg, ocr_result,
        FLAGS_output + "/" + file_name); // 这个函数里带一句cout 
    }
  }
 
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true); // 解析命令参数
  load_congif_file(); // 加载配置文件
  check_params(); // 检测参数合法性
  if (FLAGS_type != "ocr") { // 只留下ocr功能，表格识别先不管
    std::cerr << "[ERROR] type only 'ocr' is allow" << endl;
    system("pause");
    return 0;
  }

  PPOCR ocr = PPOCR(); // 初始化识别器 
  system("chcp 65001"); // 控制台设utf-8
  cout << "OCR initialization completed." << endl; // 完成提示

  // 启动参数传入图片，则执行一次性识别
  if(FLAGS_image_dir!="")
    runOCR(ocr, FLAGS_image_dir);
  else // 否则，无限循环读取图片
    while (1) {
      string img_path;
      getline(cin, img_path);
      // 去除控制台拖入文件遇到空格路径自动加的双引号 
      if (img_path[0] == '\"') {
        img_path = img_path.substr(1, img_path.length() - 2);
      }
      runOCR(ocr, img_path);
    }


  system("pause");
  return 0;
}


 /* if (!Utility::PathExists(FLAGS_image_dir)) {
    std::cerr << "[ERROR] image path not exist! image_dir: " << FLAGS_image_dir
              << endl;
    system("pause");
    exit(1);
  }

  std::vector<cv::String> cv_all_img_names;
  cv::glob(FLAGS_image_dir, cv_all_img_names);
  std::cout << "total images num: " << cv_all_img_names.size() << endl;


  if (FLAGS_type == "ocr") {
    ocr(cv_all_img_names);
  } else if (FLAGS_type == "structure") {
    structure(cv_all_img_names);
  } else {
    std::cout << "only value in ['ocr','structure'] is supported" << endl;
  }*/