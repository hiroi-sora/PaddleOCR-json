// PaddleOCR-json
// https://github.com/hiroi-sora/PaddleOCR-json

#ifndef TASK_H
#define TASK_H

namespace PaddleOCR {

// ==================== 标志码 ==================== 
#define CODE_INIT                0   // 每回合初始值，回合结束时仍为它代表受管控的区域内未发现错误
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

// ==================== 任务调用类 ==================== 
class Task {

public:
	int ocr(); // OCR图片 

private:
	PPOCR ppocr; // OCR引擎对象 
	int t_code; // 本轮任务状态码 
	std::string t_msg; // 本轮任务状态消息 

	// 任务流程
	int single_image(); // 单次识别模式 
	int socket_mode(); // 套接字模式 
	int anonymous_pipe_mode(); // 匿名管道模式 

	// 工具 
	void set_state(int code = CODE_INIT, std::string msg = ""); // 设置状态 
	std::string get_state_json(); // 获取当前状态的json字符串 

	// OpenCV相关
	//    代替cv imread，接收utf-8字符串传入，返回Mat。失败时设置错误码，并返回空Mat。 
	cv::Mat imread_u8(std::string path, int flag = cv::IMREAD_COLOR);
#ifdef _WIN32
	//    代替 cv::imread ，从路径pathW读入一张图片。pathW必须为unicode的wstring
	cv::Mat imread_wstr(std::wstring pathW, int flags = cv::IMREAD_COLOR);
#endif
	//    从剪贴板中读取图片 
	cv::Mat imread_clipboard(int flag = cv::IMREAD_COLOR);

};

} // namespace PaddleOCR

#endif // TASK_H