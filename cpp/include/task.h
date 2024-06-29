// PaddleOCR-json
// https://github.com/hiroi-sora/PaddleOCR-json

#ifndef TASK_H
#define TASK_H

#include <chrono>
#include <thread>
#include <mutex>

#include "include/nlohmann/json.hpp" // json库

namespace PaddleOCR
{

// ==================== 标志码 ====================
#define CODE_INIT 0 // 每回合初始值，回合结束时仍为它代表受管控的区域内未发现错误
// 识别成功
#define CODE_OK 100      // 成功，且识别出文字
#define CODE_OK_NONE 101 // 成功，且未识别到文字
#define MSG_OK_NONE(p) "No text found in image. Path: \"" + p + "\""
// 按路径读图，失败
#define CODE_ERR_PATH_EXIST 200 // 图片路径不存在
#define MSG_ERR_PATH_EXIST(p) "Image path dose not exist. Path: \"" + p + "\""
#define CODE_ERR_PATH_CONV 201 // 图片路径string无法转换到wstring
#define MSG_ERR_PATH_CONV(p) "Image path failed to convert to utf-16 wstring. Path: \"" + p + "\""
#define CODE_ERR_PATH_READ 202 // 图片路径存在，但无法打开文件
#define MSG_ERR_PATH_READ(p) "Image open failed. Path: \"" + p + "\""
#define CODE_ERR_PATH_DECODE 203 // 图片打开成功，但读取到的内容无法被opencv解码
#define MSG_ERR_PATH_DECODE(p) "Image decode failed. Path: \"" + p + "\""

#if defined(_WIN32) && defined(ENABLE_CLIPBOARD)
// 剪贴板读图，失败
#define CODE_ERR_CLIP_OPEN 210 // 剪贴板打开失败 ( OpenClipboard )
#define MSG_ERR_CLIP_OPEN "Clipboard open failed."
#define CODE_ERR_CLIP_EMPTY 211 // 剪贴板为空 ( GetPriorityClipboardFormat NULL )
#define MSG_ERR_CLIP_EMPTY "Clipboard is empty."
#define CODE_ERR_CLIP_FORMAT 212 // 剪贴板的格式不支持 ( GetPriorityClipboardFormat -1 )
#define MSG_ERR_CLIP_FORMAT "Clipboard format is not valid."
#define CODE_ERR_CLIP_DATA 213 // 剪贴板获取内容句柄失败，通常由别的程序占用剪贴板引起 ( GetClipboardData NULL )
#define MSG_ERR_CLIP_DATA "Getting clipboard data handle failed."
#define CODE_ERR_CLIP_FILES 214 // 剪贴板查询到的文件的数量不为1 ( DragQueryFile != 1 )
#define MSG_ERR_CLIP_FILES(n) "Clipboard number of query files is not valid. Number: " + std::to_string(n)
#define CODE_ERR_CLIP_GETOBJ 215 // 剪贴板检索图形对象信息失败 ( GetObject NULL )
#define MSG_ERR_CLIP_GETOBJ "Clipboard get bitmap object failed."
#define CODE_ERR_CLIP_BITMAP 216 // 剪贴板获取位图数据失败 ( GetBitmapBits 复制字节为空 )
#define MSG_ERR_CLIP_BITMAP "Getting clipboard bitmap bits failed."
#define CODE_ERR_CLIP_CHANNEL 217 // 剪贴板中位图的通道数不支持 ( nChannels 不为1，3，4 )
#define MSG_ERR_CLIP_CHANNEL(n) "Clipboard number of image channels is not valid. Number: " + std::to_string(n)
#endif

// base64读图，失败
#define CODE_ERR_BASE64_DECODE 300 // base64字符串解析为string失败 
#define MSG_ERR_BASE64_DECODE "Base64 decode failed."
#define CODE_ERR_BASE64_IM_DECODE 301 //  base64字符串解析成功，但读取到的内容无法被opencv解码 
#define MSG_ERR_BASE64_IM_DECODE "Base64 data imdecode failed."
// json相关
#define CODE_ERR_JSON_DUMP 400 // json对象 转字符串失败 
#define MSG_ERR_JSON_DUMP "Json dump failed."
#define CODE_ERR_JSON_PARSE 401 // json字符串 转对象失败 
#define MSG_ERR_JSON_PARSE "Json parse failed."
#define CODE_ERR_JSON_PARSE_KEY 402 // json对象 解析某个键时失败 
#define MSG_ERR_JSON_PARSE_KEY(k) "Json parse key [" + k + "] failed."
#define CODE_ERR_NO_TASK 403 // 未发现有效任务 
#define MSG_ERR_NO_TASK "No valid tasks."

    // ==================== 任务调用类 ====================
    class Task
    {

    public:
        // 使用默认constructor
        Task() = default;
        // 自定义destructor，会自动释放指针和线程
        ~Task();
        // 禁用复制和赋值
        // class Task底下有PPOCR指针，还是多线程的class，不应该被复制。
        // 可以直接用 `Task task;` 来创建对象。
        Task(Task&) = delete;
        Task(Task&&) = delete;
        Task& operator=(Task&) = delete;
        Task& operator=(Task&&) = delete;
        
        int ocr(); // OCR图片

    private:
        bool is_exit = false;   // 为true时退出任务循环 
        PPOCR *ppocr = nullptr; // OCR引擎指针 
        int t_code;             // 本轮任务状态码 
        std::string t_msg;      // 本轮任务状态消息 
        // 上一次结束OCR的时间
        std::chrono::high_resolution_clock::time_point last_active_time;
        bool is_active = false;     // 是否正在进行OCR
        std::mutex mutex;           // 互斥锁
        std::thread cleanup_thread; // 内存清理线程

    private:
        // 任务流程
        std::string run_ocr(std::string); // 输入用户传入值（字符串），返回结果json字符串
        int single_image_mode();   // 单次识别模式 
        int socket_mode();         // 套接字模式 
        int anonymous_pipe_mode(); // 匿名管道模式 

        // 输出相关 
        void set_state(int code = CODE_INIT, std::string msg = "");             // 设置状态 
        std::string get_state_json(int code = CODE_INIT, std::string msg = ""); // 获取状态json字符串 
        std::string get_ocr_result_json(const std::vector<OCRPredictResult> &); // 传入OCR结果，返回json字符串 

        // 输入相关 
        std::string json_dump(nlohmann::json); // json对象转字符串 
        cv::Mat imread_json(std::string &);  // 输入json字符串，解析json并返回图片Mat 
        cv::Mat imread_u8(std::string path, int flag = cv::IMREAD_COLOR); // 代替cv imread，输入utf-8字符串，返回Mat。失败时设置错误码，并返回空Mat。 
        cv::Mat imread_clipboard(int flag = cv::IMREAD_COLOR); // 从当前剪贴板中读取图片 
        cv::Mat imread_base64(std::string&, int flag = cv::IMREAD_COLOR); // 输入base64编码的字符串，返回Mat 
#ifdef _WIN32
        cv::Mat imread_wstr(std::wstring pathW, int flags = cv::IMREAD_COLOR); // 输入unicode wstring字符串，返回Mat。 
#endif
        
        // 其他
        
        // ipv4 地址转 uint32_t
        int addr_to_uint32(const std::string& addr, uint32_t& addr_out);
        
        // 内存清理相关
        void cleanup_ppocr_if_needed();
        void cleanup_thread_loop(int check_intval);
        void cleanup_thread_start();
        void cleanup_thread_join();
    };

} // namespace PaddleOCR

#endif // TASK_H