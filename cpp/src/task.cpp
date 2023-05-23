
#include <include/paddleocr.h>
#include <include/args.h>
#include <include/task.h>

namespace PaddleOCR {

// OCR 任务循环
int Task::ocr(){
	// 初始化引擎
	ppocr = PPOCR();
	std::cout << "OCR init completed." << std::endl;

	// 单张图片识别模式 
	if (!FLAGS_image_path.empty()) {
		return single_image();
	}
	//// 套接字服务器模式 
	//else if (FLAGS_port != -1) {
	//	return socket_mode();
	//}
	//// 匿名管道模式
	//else {
	//	return anonymous_pipe_mode();
	//}

	return 0;
}

// 设置状态
void Task::set_state(int code, std::string msg) {
	t_code = code;
	t_msg = msg;
}

// 获取状态json 
std::string get_state_json() {

}

// 单张图片识别模式
int Task::single_image() {
	set_state();
	cv::Mat img = imread_u8(FLAGS_image_path);
	if (img.empty()) { // 读图失败 
		std::string str = get_state_json(); // 获取状态json 
		return 0;
	}
	//if (!PaddleOCR::Utility::PathExists(FLAGS_image_path)) {
	//	set_state(CODE_ERR_PATH_EXIST, MSG_ERR_PATH_EXIST(FLAGS_image_path)); // 报告状态：路径不存在 
	//	return 1;
	//}


	return 0;
}

}