
#include <exception>
#include <regex>

#include "include/paddleocr.h"
#include "include/args.h"
#include "include/task.h"
#include "include/base64.h" // base64库

// htonl 函数
#if defined(_WIN32)
#include <windows.h>
#else // Linux, Mac
#include <arpa/inet.h>
#endif

namespace PaddleOCR
{
    // ==================== 工具 ====================

    // json对象转字符串
    std::string Task::json_dump(nlohmann::json j)
    {
        try
        {
            std::string str = j.dump(-1, ' ', FLAGS_ensure_ascii);
            return str;
        }
        catch (...)
        {
            nlohmann::json j2;
            j2["code"] = CODE_ERR_JSON_DUMP;
            j2["data"] = MSG_ERR_JSON_DUMP;
            std::string str = j2.dump(-1, ' ', FLAGS_ensure_ascii);
            return str;
        }
    }

    // 设置状态
    void Task::set_state(int code, std::string msg)
    {
        t_code = code;
        t_msg = msg;
    }

    // 获取状态json字符串
    std::string Task::get_state_json(int code, std::string msg)
    {
        nlohmann::json j;
        if (code == CODE_INIT && msg.empty())
        { // 留空，填充当前状态
            code = t_code;
            msg = t_msg;
        }
        j["code"] = code;
        j["data"] = msg;
        return json_dump(j);
    }
    
    // 将OCR结果转换为json字符串  
    std::string Task::get_ocr_result_json(const std::vector<OCRPredictResult> &ocr_result)
    {
        nlohmann::json outJ;
        outJ["code"] = 100;
        outJ["data"] = nlohmann::json::array();
        bool isEmpty = true;
        for (int i = 0; i < ocr_result.size(); i++)
        {
            nlohmann::json j;
            j["text"] = ocr_result[i].text;
            j["score"] = ocr_result[i].score;
            std::vector<std::vector<int>> b = ocr_result[i].box;
            // 无包围盒
            if (b.empty())
            {
                if (FLAGS_det) // 开了det仍无包围盒，跳过本组 
                    continue;
                else // 未开det，填充空包围盒 
                    for (int bi = 0; bi < 4; bi++)
                        b.push_back(std::vector<int>{-1, -1});
            }
            // 启用了rec仍没有文字，跳过本组 
            if (FLAGS_rec && (j["score"] <= 0 || j["text"] == ""))
            {
                continue;
            }
            else
            {
                j["box"] = {{b[0][0], b[0][1]}, {b[1][0], b[1][1]}, {b[2][0], b[2][1]}, {b[3][0], b[3][1]}};
            }
            outJ["data"].push_back(j);
            isEmpty = false;
        }
        // 结果1：识别成功，无文字（rec未检出） 
        if (isEmpty)
        {
            return "";
        }
        // 结果2：识别成功，有文字 
        return json_dump(outJ);
    }

    // 输入base64编码的字符串，返回Mat
    cv::Mat Task::imread_base64(std::string& b64str, int flag) {
        std::string decoded_string;
        try {
            decoded_string = base64_decode(b64str);
        }
        catch (...) {
            set_state(CODE_ERR_BASE64_DECODE, MSG_ERR_BASE64_DECODE); // 报告状态：解析失败 
            return cv::Mat();
        }
        try {
            std::vector<uchar> data(decoded_string.begin(), decoded_string.end());
            cv::Mat img = cv::imdecode(data, flag);
            if (img.empty()) {
                set_state(CODE_ERR_BASE64_IM_DECODE, MSG_ERR_BASE64_IM_DECODE); // 报告状态：转Mat失败 
            }
            return img;
        }
        catch (...) {
            set_state(CODE_ERR_BASE64_IM_DECODE, MSG_ERR_BASE64_IM_DECODE); // 报告状态：转Mat失败 
            return cv::Mat();
        }
    }

    // 输入json字符串，解析并读取Mat 
    cv::Mat Task::imread_json(std::string& str_in) {
#ifdef ENABLE_REMOTE_EXIT
        if (str_in == "exit") { // 退出指令 
            is_exit = true;
            return cv::Mat();
        }
#endif
        cv::Mat img;
        bool is_image_found = false; // 当前是否已找到图片 
        std::string logstr = "";
        // 解析为json对象 
        auto j = nlohmann::json();
        try {
            j = nlohmann::json::parse(str_in); // 转json对象 
        }
        catch (...) {
            set_state(CODE_ERR_JSON_PARSE, MSG_ERR_JSON_PARSE); // 报告状态：解析失败 
            return cv::Mat();
        }
        for (auto& el : j.items()) { // 遍历键值 
#ifdef ENABLE_REMOTE_EXIT
            if (el.key() == "exit") { // 退出指令 
                is_exit = true;
                return cv::Mat();
            }
#endif
            try {
                std::string value = to_string(el.value());
                int vallen = value.length();
                if (vallen > 2 && value[0] == '\"' && value[vallen - 1] == '\"') {
                    value = value.substr(1, vallen - 2); // 删去nlohmann字符串的两端引号
                }
                // 提取图片 
                if (!is_image_found) {
                    if (el.key() == "image_base64") { // base64字符串 
                        FLAGS_image_path = "base64"; // 设置图片路径标记，以便于无文字时的信息输出 
                        img = imread_base64(value); // 读取图片 
                        is_image_found = true;
                    }
#ifdef ENABLE_JSON_IMAGE_PATH
                    else if (el.key() == "image_path") { // 图片路径
                        FLAGS_image_path = value;
                        img = imread_u8(value); // 读取图片 
                        is_image_found = true;
                    }
#endif
                }
                //else {} // TODO: 其它参数热更新
            }
            catch (...) { // 安全起见，出现未知异常时结束本轮任务 
                set_state(CODE_ERR_JSON_PARSE_KEY, MSG_ERR_JSON_PARSE_KEY(el.key())); // 报告状态：解析键失败 
                return cv::Mat();
            }
        }
        if (!is_image_found) {
            set_state(CODE_ERR_NO_TASK, MSG_ERR_NO_TASK); // 报告状态：未发现有效任务 
        }
        return img;
    }


    // ==================== 任务流程 ====================

    std::string Task::run_ocr(std::string str_in) {
        cv::Mat img = imread_json(str_in);
        if (is_exit) { // 退出 
            return "";
        }
        if (img.empty()) { // 读图失败 
            return get_state_json();
        }
        // 执行OCR
        std::vector<OCRPredictResult> res_ocr;
        { 
            const std::lock_guard<std::mutex> lock(mutex);
            
            // 更新OCR状态：正在运行
            is_active = true;
            
            // OCR
            res_ocr = ppocr->ocr(img, FLAGS_det, FLAGS_rec, FLAGS_cls);
            
            // 更新OCR状态和时间：已完成OCR
            is_active = false;
            last_active_time = std::chrono::high_resolution_clock::now();
        }
        // 获取结果 
        std::string res_json = get_ocr_result_json(res_ocr);
        // 结果1：识别成功，无文字（rec未检出）
        if (res_json.empty()) {
            return get_state_json(CODE_OK_NONE, MSG_OK_NONE(FLAGS_image_path));
        }
        // 结果2：识别成功，有文字 
        else {
            return res_json;
        }
    }

    Task::~Task()
    {
        if (ppocr)
        {
            delete ppocr;
            ppocr = nullptr;
        }
        
        cleanup_thread_join();
    }
    
    // 入口
    int Task::ocr()
    {
        // 初始化引擎
        ppocr = new PPOCR(); // 创建引擎对象
        int flag;
        last_active_time = std::chrono::high_resolution_clock::now();
        
#if defined(_WIN32) && defined(ENABLE_CLIPBOARD)
        std::cout << "OCR clipboard enbaled." << std::endl;
#endif
        
        // 单张图片识别模式
        if (!FLAGS_image_path.empty()){
            std::cout << "OCR single image mode. Path: " << FLAGS_image_path << std::endl;
            flag = 1;
        }
        // 套接字服务器模式
        else if (FLAGS_port >= 0 && !FLAGS_addr.empty()) {
            std::cout << "OCR socket mode. Addr: " << FLAGS_addr  << ", Port: " << FLAGS_port << std::endl;
            flag = 2;
        }
        // 匿名管道模式
        else {
            std::cout << "OCR anonymous pipe mode." << std::endl;
            flag = 3;
        }
        std::cout << "OCR init completed." << std::endl;
        
        switch (flag)
        {
        case 1:
            return single_image_mode();
        case 2:
            return socket_mode();
        case 3:
            return anonymous_pipe_mode();
        }
        return 0;
    }
    
    // 单张图片识别模式 
    int Task::single_image_mode()
    {
        set_state();
        cv::Mat img = imread_u8(FLAGS_image_path);
        if (img.empty())
        { // 读图失败 
            std::cout << get_state_json() << std::endl;
            return 0;
        }
        // 执行OCR
        // 这里只会运行一次OCR，就不用做内存清理了。
        std::vector<OCRPredictResult> res_ocr = ppocr->ocr(img, FLAGS_det, FLAGS_rec, FLAGS_cls);
        // 获取结果 
        std::string res_json = get_ocr_result_json(res_ocr);
        // 结果1：识别成功，无文字（rec未检出）
        if (res_json.empty()) {
            std::cout << get_state_json(CODE_OK_NONE, MSG_OK_NONE(FLAGS_image_path)) << std::endl;
        }
        // 结果2：识别成功，有文字 
        else {
            std::cout << res_json << std::endl;
        }
        return 0;
    }
    
    // 匿名管道模式 
    int Task::anonymous_pipe_mode() {
        cleanup_thread_start(); // 启动内存清理线程
        while (1) {
            set_state(); // 初始化状态 
            // 读取一行输入 
            std::string str_in;
            getline(std::cin, str_in);
            // 获取ocr结果并输出 
            std::string str_out = run_ocr(str_in);
            if (is_exit) { // 退出
                cleanup_thread_join();
                return 0;
            }
            std::cout << str_out << std::endl;
        }
        cleanup_thread_join(); // 结束内存清理线程
        return 0;
    }
    
    // 套接字服务器模式，在平台内定义 
    
    
    // 其他函数
    
    // ipv4 地址转 uint32_t
    int Task::addr_to_uint32(const std::string& addr, uint32_t& addr_out)
    {
        // 处理特殊情况
        if (addr == "loopback" || addr == "localhost")
        {
            addr_out = htonl(INADDR_LOOPBACK);
            return 0;
        }
        else if (addr == "any")
        {
            addr_out = htonl(INADDR_ANY);
            return 0;
        }
        
        // 使用正则表达式来处理IPv4地址
        std::regex rgx(R"((\d+)\.(\d+)\.(\d+)\.(\d+))");
        std::smatch matches;
        uint32_t output = 0;
        
        // 如果验证为IPv4地址，将其转成 uint32_t 主机字节序
        if(std::regex_search(addr, matches, rgx))
        {
            uint8_t octet;
            for (size_t i = 1; i < matches.size(); ++i)
            {
                octet = static_cast<uint8_t>(std::stoi(matches[i].str()));
                output |= octet << (8 * (4-i));
            }
        }
        // 反之则报错
        else
        {
            return -1;
        }
        
        // 最后把 uint32_t 主机字节序 转成 网络字节序
        addr_out = htonl(output);
        return 0;
    }
    
    // 内存清理相关
    void Task::cleanup_ppocr_if_needed()
    {
        const std::lock_guard<std::mutex> lock(mutex);
        auto now = std::chrono::high_resolution_clock::now();
        auto time_interval = std::chrono::duration_cast<std::chrono::seconds>(now - last_active_time);
        
        if (FLAGS_auto_memory_cleanup > 0 // 启用自动内存清理
            && !is_active // 并且当前没有OCR工作
            && time_interval.count() > FLAGS_auto_memory_cleanup // 并且现在距离上次OCR结束的时间间隔大于参数的值
        )
        {
            // 清理内存
            delete ppocr; // 清理引擎对象
            ppocr = new PPOCR(); // 重新创建引擎对象
            // 更新时间
            last_active_time = std::chrono::high_resolution_clock::now();
        }
    }
    
    void Task::cleanup_thread_loop(int check_interval)
    {
        // 如果启用自动内存清理
        while (FLAGS_auto_memory_cleanup > 0)
        {
            std::this_thread::sleep_for(std::chrono::seconds(check_interval));
            cleanup_ppocr_if_needed();
        }
    }
    
    void Task::cleanup_thread_start()
    {
        // 如果启用自动内存清理
        if (FLAGS_auto_memory_cleanup > 0)
        {
            // 使用内存清理时间参数来计算清理检查的间隔时间
            // 使用参数的 1/10 来减少不必要的检查
            int check_interval = static_cast<int>(FLAGS_auto_memory_cleanup * 0.1);
            cleanup_thread = std::thread(&Task::cleanup_thread_loop, this, check_interval);
        }
    }
    
    void Task::cleanup_thread_join()
    {
        // 如果启用自动内存清理
        if (FLAGS_auto_memory_cleanup > 0)
        {
            while (!cleanup_thread.joinable())
                std::this_thread::sleep_for(std::chrono::seconds(1));
            cleanup_thread.join();
        }
    }
    

}

// ./PaddleOCR-json.exe -config_path="models/zh_CN.txt" -image_path="D:/Test/t2.png" -ensure_ascii=0