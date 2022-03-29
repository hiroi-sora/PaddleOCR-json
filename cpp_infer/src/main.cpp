#include "glog/logging.h"
#include "omp.h"
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <vector>

#include <cstring>
#include <fstream>
#include <numeric>

#include <include/config.h>
#include <include/ocr_det.h>
#include <include/ocr_rec.h>

#include <include/configor/json.hpp> // json库 
using namespace configor;

using namespace std;
using namespace cv;
using namespace PaddleOCR;

int main(int argc, char* argv[])
{
    system("chcp 65001"); // 控制台转utf-8编码 
    // 获取程序名称，根据名称选择对应的配置 
    string argvName = argv[0];
    string::size_type xPos = argvName.rfind('\\') + 1; // 最后一个斜杠位置 
    string::size_type dPos = argvName.rfind('.'); // 最后一个点位置
    string fileName = argvName.substr(xPos, dPos - xPos); // 裁剪出exe文件名 
    std::string config_path = fileName+"_config.txt"; // 配置文件名 
    // 载入配置 
    OCRConfig config((config_path.c_str()));
    //config.PrintConfigInfo(); // 输出配置 
    DBDetector det(config.det_model_dir, config.use_gpu, config.gpu_id,
        config.gpu_mem, config.cpu_math_library_num_threads,
        config.use_mkldnn, config.max_side_len, config.det_db_thresh,
        config.det_db_box_thresh, config.det_db_unclip_ratio,
        config.use_polygon_score, config.visualize,
        config.use_tensorrt, config.use_fp16);
    Classifier* cls = nullptr;
    if (config.use_angle_cls == true) // 启用倾斜角度 
    {
        cls = new Classifier(config.cls_model_dir, config.use_gpu, config.gpu_id,
            config.gpu_mem, config.cpu_math_library_num_threads,
            config.use_mkldnn, config.cls_thresh,
            config.use_tensorrt, config.use_fp16);
    }
    // 载入模型
    CRNNRecognizer rec(config.rec_model_dir, config.use_gpu, config.gpu_id,
        config.gpu_mem, config.cpu_math_library_num_threads,
        config.use_mkldnn, config.char_list_file,
        config.use_tensorrt, config.use_fp16);

    std::string img_path; 
    while(getline(cin, img_path)) { // 读取图片路径，读到空行退出 
        // 去除控制台遇到空格路径自动加的双引号 
        if (img_path[0] == '\"') {
            img_path = img_path.substr(1, img_path.length() - 2);
        }

        // 载入图片 
        cv::Mat srcimg = cv::imread(img_path, cv::IMREAD_COLOR);
        if (!srcimg.data) // 读入图片失败 
        {
            json errjson = { {"error", "读取图片失败  " + img_path} };
            std::cout << errjson << endl;
            continue;
        }
        
        // 运行 
        std::vector<std::vector<std::vector<int>>> boxes; // 识别出的包围盒 
        std::vector<std::string> out_str; // 识别出的字符串 
        std::vector<float> out_score; // 识别字符串的置信度 
        det.Run(srcimg, boxes); 
        rec.Run(boxes, srcimg, cls, out_str, out_score);

        // 转json 
        json textArr = json::array({});
        int s = out_str.size(); 
        for (int i = 0; i < s; i++) {
            json textObj = {
                    {"text", out_str[i]},
                    {"score", out_score[i]},
                    {"box", { boxes[i][0][0], boxes[i][0][1], boxes[i][1][0], boxes[i][1][1], boxes[i][2][0], boxes[i][2][1], boxes[i][3][0], boxes[i][3][1] },
                }
            };
            textArr[s-i-1] = textObj;
        }
        std::cout << textArr << endl;
    }
    return 0;
}