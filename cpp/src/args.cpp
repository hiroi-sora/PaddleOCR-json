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

#include <string>
#include <fstream>
#include <include/utility.h>

#include <gflags/gflags.h>

// 工作模式
DEFINE_string(image_path, "", "Set image_path to run a single task."); // 若填写了图片路径，则执行一次OCR。
DEFINE_int32(port, -1, "Set to 0 enable random port, set to 1~65535 enables specified port.");      // 填写0随机端口号，填1^65535指定端口号。默认则启用匿名管道模式。
DEFINE_string(addr, "loopback", "Socket server addr, the value can be 'loopback', 'localhost', 'any', or other IPv4 address."); // 套接字服务器的地址模式，本地环回/任何可用。 
// 设置无OCR工作时自动清理内存来减少占用。值 <= 0 时禁用功能。 值 > 0 时将值作为时间（秒），每当无OCR工作一段时间后就清理一次内存。默认值 1800 秒（30分钟）。
DEFINE_int32(auto_memory_cleanup, 1800, "Set automatically cleanup memory usage when there is no ocr jobs. Value <= 0 to disable. Value > 0 means value as seconds, run a cleanup after no ocr job for specified time. Default 1800 seconds (30 minutes).");

// common args 常用参数
DEFINE_bool(use_gpu, false, "Infering with GPU or CPU.");                                              // true时启用GPU（需要推理库支持）
DEFINE_bool(use_tensorrt, false, "Whether use tensorrt.");                                             // true时启用tensorrt
DEFINE_int32(gpu_id, 0, "Device id of GPU to execute.");                                               // GPU id，使用GPU时有效
DEFINE_int32(gpu_mem, 4000, "GPU memory when infering with GPU.");                                     // 申请的GPU内存
DEFINE_int32(cpu_threads, 10, "Num of threads with CPU.");                                             // CPU线程
DEFINE_bool(enable_mkldnn, true, "Whether use mkldnn with CPU.");                                     // true时启用mkldnn
DEFINE_string(precision, "fp32", "Precision be one of fp32/fp16/int8");                                // 预测的精度，支持fp32, fp16, int8 3种输入
DEFINE_bool(benchmark, false, "Whether use benchmark.");                                               // true时开启benchmark，对预测速度、显存占用等进行统计
DEFINE_string(output, "./output/", "Save benchmark log path.");                                        // 可视化结果保存的路径 TODO
DEFINE_string(type, "ocr", "Perform ocr or structure, the value is selected in ['ocr','structure']."); // 任务类型（暂不可用）
DEFINE_string(config_path, "", "Path of config file.");                                                // 配置文件路径
DEFINE_string(models_path, "", "Path of models folder.");                                              // 预测库路径
DEFINE_bool(ensure_ascii, true, "Enable JSON ascii escape.");                                          // true时json开启ascii转义

// detection related DET检测相关
DEFINE_string(det_model_dir, "models/ch_PP-OCRv3_det_infer", "Path of det inference model.");                                                 // det模型库路径
DEFINE_string(limit_type, "max", "limit_type of input image, the value is selected in ['max','min'].");           // 对图片尺寸限制采用长边还是短边
DEFINE_int32(limit_side_len, 960, "limit_side_len of input image.");                                              // 对长/短边限制值
DEFINE_double(det_db_thresh, 0.3, "Threshold of det_db_thresh.");                                                 // 用于过滤DB预测的二值化图像，设置为0.-0.3对结果影响不明显
DEFINE_double(det_db_box_thresh, 0.6, "Threshold of det_db_box_thresh.");                                         // DB后处理过滤box的阈值，如果检测存在漏框情况，可酌情减小
DEFINE_double(det_db_unclip_ratio, 1.5, "Threshold of det_db_unclip_ratio.");                                     // 表示文本框的紧致程度，越小则文本框更靠近文本
DEFINE_bool(use_dilation, false, "Whether use the dilation on output map.");                                      // true时对分割结果进行膨胀以获取更优检测效果
DEFINE_string(det_db_score_mode, "slow", "Whether use polygon score, the value is selected in ['slow','fast']."); // slow:使用多边形框计算bbox score，fast:使用矩形框计算。矩形框计算速度更快，多边形框对弯曲文本区域计算更准确
DEFINE_bool(visualize, false, "Whether show the detection results.");                                             // true时启用结果进行可视化，预测结果保存在output字段指定的文件夹下和输入图像同名的图像上。

// classification related CLS方向分类相关
DEFINE_bool(use_angle_cls, false, "Whether use use_angle_cls.");  // true时启用方向分类器
DEFINE_string(cls_model_dir, "models/ch_ppocr_mobile_v2.0_cls_infer", "Path of cls inference model.");
DEFINE_double(cls_thresh, 0.9, "Threshold of cls_thresh.");       // 方向分类器的得分阈值
DEFINE_int32(cls_batch_num, 1, "cls_batch_num.");                 // 方向分类器batchsize

// recognition related REC文本识别相关
DEFINE_string(rec_model_dir, "models/ch_PP-OCRv3_rec_infer", "Path of rec inference model.");
DEFINE_int32(rec_batch_num, 6, "rec_batch_num.");                                                // 文字识别模型batchsize
DEFINE_string(rec_char_dict_path, "models/dict_chinese.txt", "Path of dictionary."); // 字典路径
DEFINE_int32(rec_img_h, 48, "rec image height");                                                 // 文字识别模型输入图像高度。V3模型是48，V2应该改为32
DEFINE_int32(rec_img_w, 320, "rec image width");                                                 // 文字识别模型输入图像宽度。V3和V2一致

// layout model related 版面分析相关
DEFINE_string(layout_model_dir, "", "Path of table layout inference model.");
DEFINE_string(layout_dict_path, "../../ppocr/utils/dict/layout_dict/layout_publaynet_dict.txt", "Path of dictionary."); // 版面字典路径
DEFINE_double(layout_score_threshold, 0.5, "Threshold of score.");                                                      // 检测框的分数阈值
DEFINE_double(layout_nms_threshold, 0.5, "Threshold of nms.");                                                          // nms的阈值
// structure model related 表格结构相关
DEFINE_string(table_model_dir, "", "Path of table struture inference model.");
DEFINE_int32(table_max_len, 488, "max len size of input image.");              // 表格识别模型输入图像长边大小
DEFINE_int32(table_batch_num, 1, "table_batch_num.");
DEFINE_bool(merge_no_span_structure, true, "Whether merge <td> and </td> to <td></td>");                          // true时将<td>和</td>合并到<td></td>
DEFINE_string(table_char_dict_path, "../../ppocr/utils/dict/table_structure_dict_ch.txt", "Path of dictionary."); // 表格字典路径

// ocr forward related 前处理相关
DEFINE_bool(det, true, "Whether use det in forward.");
DEFINE_bool(rec, true, "Whether use rec in forward.");
DEFINE_bool(cls, false, "Whether use cls in forward.");
DEFINE_bool(table, false, "Whether use table structure in forward.");
DEFINE_bool(layout, false, "Whether use layout analysis in forward.");


// 检查一个路径path是否存在，将信息写入msg
void check_path(const std::string& path, const std::string& name, std::string& msg)
{
    if (path.empty()){
        msg += (name + " is empty. ");
    }
    else if (!PaddleOCR::Utility::PathExists(path)) {
        msg += (name + " [" + path + "] does not exist. ");
    }
}

// 为 value 前置拼接预测库路径
void prepend_models(const std::string& models_path_base, std::string& value)
{
    if (PaddleOCR::Utility::str_starts_with(value, "models")) {
        value.erase(0, 6);
        value = PaddleOCR::Utility::pathjoin(models_path_base, value);
    }
}

// 从配置文件中读取配置，返回日志字符串。
std::string read_config()
{
    // 设置默认预测库路径
    std::string models_path_base = "models";
    // 如果输入正常预测库路径参数
    if (!FLAGS_models_path.empty() && PaddleOCR::Utility::PathExists(FLAGS_models_path))
    {
        // 则更新预测库路径
        models_path_base = FLAGS_models_path;
        // 之后我们会用这个预测库路径来更新所有其他参数的路径
    }
    
    if (!PaddleOCR::Utility::PathExists(FLAGS_config_path))
    {
        return ("config_path [" + FLAGS_config_path + "] does not exist. ");
    }
    std::ifstream infile(FLAGS_config_path);
    if (!infile)
    {
        return ("[WARNING] Unable to open config_path [" + FLAGS_config_path + "]. ");
    }
    std::string msg = "Load config from [" + FLAGS_config_path + "] : ";
    std::string line;
    int num = 0;
    while (getline(infile, line))
    {
        int length = line.length();
        if (length < 3 || line[0] == '#') // 跳过空行和注释
            continue;
        int split = 0; // 键值对的分割线
        for (; split < length; split++)
        {
            if (line[split] == ' ' || line[split] == '=')
                break;
        }
        if (split >= length - 1 || split == 0) // 跳过长度不足的键值对
            continue;
        std::string key = line.substr(0, split);
        std::string value = line.substr(split + 1);
        prepend_models(models_path_base, value);
        // 设置配置，优先级低于命令行传入参数。
        std::string res = google::SetCommandLineOptionWithMode(key.c_str(), value.c_str(), google::SET_FLAG_IF_DEFAULT);
        if (!res.empty())
        {
            num++;
            msg += res.substr(0, res.length() - 1);
        }
    }
    infile.close();
    if (num == 0)
        msg += "No valid config found.";
    else
        msg += ". ";
    return msg;
}

// 检测参数合法性。成功返回空字符串，失败返回报错信息字符串。
std::string check_flags() {
    // 设置默认预测库路径
    std::string models_path_base = "models";
    // 如果输入正常预测库路径参数
    if (!FLAGS_models_path.empty() && PaddleOCR::Utility::PathExists(FLAGS_models_path))
    {
        // 则更新预测库路径
        models_path_base = FLAGS_models_path;
        // 之后我们会用这个预测库路径来更新所有其他参数的路径
    }
    
    std::string msg = "";
    if (FLAGS_det) { // 检查det
        prepend_models(models_path_base, FLAGS_det_model_dir);
        check_path(FLAGS_det_model_dir, "det_model_dir", msg);
    }
    if (FLAGS_rec) { // 检查rec
        prepend_models(models_path_base, FLAGS_rec_model_dir);
        check_path(FLAGS_rec_model_dir, "rec_model_dir", msg);
    }
    if (FLAGS_cls && FLAGS_use_angle_cls) { // 检查cls
        prepend_models(models_path_base, FLAGS_cls_model_dir);
        check_path(FLAGS_cls_model_dir, "cls_model_dir", msg);
    }
    if (!FLAGS_rec_char_dict_path.empty()) { // 检查 rec_char_dict_path
        prepend_models(models_path_base, FLAGS_rec_char_dict_path);
        check_path(FLAGS_rec_char_dict_path, "rec_char_dict_path", msg);
    }
    if (FLAGS_table) { // 检查table
        prepend_models(models_path_base, FLAGS_table_model_dir);
        check_path(FLAGS_table_model_dir, "table_model_dir", msg);
        if (!FLAGS_det)
            check_path(FLAGS_det_model_dir, "det_model_dir", msg);
        if (!FLAGS_rec)
            check_path(FLAGS_rec_model_dir, "rec_model_dir", msg);
    }
    if (FLAGS_layout) { // 布局
        prepend_models(models_path_base, FLAGS_layout_model_dir);
        check_path(FLAGS_layout_model_dir, "layout_model_dir", msg);
    }
    if (!FLAGS_config_path.empty()) { // 配置文件目录非空时检查存在 
        check_path(FLAGS_config_path, "config_path", msg);
    }
    // 检查枚举值
    if (FLAGS_precision != "fp32" && FLAGS_precision != "fp16" && FLAGS_precision != "int8") {
        msg += "precison should be 'fp32'(default), 'fp16' or 'int8', not " + FLAGS_precision + ". ";
    }
    if (FLAGS_type != "ocr" && FLAGS_type != "structure") {
        msg += "type should be 'ocr'(default) or 'structure', not " + FLAGS_type + ". ";
    }
    if (FLAGS_limit_type != "max" && FLAGS_limit_type != "min") {
        msg += "limit_type should be 'max'(default) or 'min', not " + FLAGS_limit_type + ". ";
    }
    if (FLAGS_det_db_score_mode != "slow" && FLAGS_det_db_score_mode != "fast") {
        msg += "limit_type should be 'slow'(default) or 'fast', not " + FLAGS_det_db_score_mode + ". ";
    }
    return msg;
}