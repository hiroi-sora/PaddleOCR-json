// Linux 平台的任务处理代码

#if defined(_LINUX) || defined(__linux__)

#include <iostream>
#include <fstream>

#include "include/paddleocr.h"
#include "include/args.h"
#include "include/task.h"

// #include <平台相关的库>

namespace PaddleOCR
{
    // 代替cv imread，接收utf-8字符串传入，返回Mat。
    cv::Mat Task::imread_u8(std::string pathU8, int flag)
    {
        if (pathU8 == "clipboard") { // 若为剪贴板任务 
            return imread_clipboard(flag);
        }
        
        std::ifstream fileInput(pathU8, std::ios::binary);
        // 路径不存在且无法输出
        if (!fileInput)
        {
            set_state(CODE_ERR_PATH_EXIST, MSG_ERR_PATH_EXIST(pathU8));
            return cv::Mat();
        }
        
        // 获取文件大小并将文件读到内存
        fileInput.seekg (0, fileInput.end);
        size_t fileLength = fileInput.tellg();
        fileInput.seekg (0, fileInput.beg);
        char *buffer = new char[fileLength];
        fileInput.read(buffer, fileLength);
        
        // 无法读取
        if (!fileInput)
        {
            set_state(CODE_ERR_PATH_READ, MSG_ERR_PATH_READ(pathU8));
            return cv::Mat();
        }
        
        // 解码内存数据，变成cv::Mat数据 
        cv::_InputArray array(buffer, fileLength);
        cv::Mat image = cv::imdecode(array, flag);
        // 解码失败
        if (image.empty())
        {
            set_state(CODE_ERR_PATH_DECODE, MSG_ERR_PATH_DECODE(pathU8));
            return cv::Mat();
        }
        // 释放buffer空间必须放在 cv::imdecode() 之后，
        // 因为 cv::_InputArray() 并不会复制buffer内的元素，
        // 而 cv::imdecode() 在解码时会新开辟一块内存并复制解码后的数据
        delete[] buffer;
        fileInput.close();
        
        return image;
    }
    
    cv::Mat Task::imread_clipboard(int flag)
    {
        std::cerr << "PaddleOCR::Task::imread_clipboard() stub\n";
        return cv::Mat();
    }
    
    int Task::socket_mode()
    {
        std::cerr << "PaddleOCR::Task::socket_mode() stub\n";
        return 0;
    }
}

#endif