// Linux 平台的任务处理代码

#if defined(_LINUX) || defined(__linux__)

#include <iostream>

#include "include/paddleocr.h"
#include "include/args.h"
#include "include/task.h"

// #include <平台相关的库>

namespace PaddleOCR
{
    cv::Mat Task::imread_u8(std::string pathU8, int flag){
        std::cerr << "PaddleOCR::Task::imread_u8() stub\n";
        return cv::Mat();
    }

    cv::Mat Task::imread_clipboard(int flag){
        std::cerr << "PaddleOCR::Task::imread_clipboard() stub\n";
        return cv::Mat();
    }

    int Task::socket_mode(){
        std::cerr << "PaddleOCR::Task::socket_mode() stub\n";
        return 0;
    }
}

#endif