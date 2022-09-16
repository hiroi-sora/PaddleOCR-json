// Copyright (c) 2022 hiroi-sora. All Rights Reserved.
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

// https://github.com/hiroi-sora/PaddleOCR-json


#include <fstream>
#include <iostream>
#include <windows.h>
#include <filesystem>
#include <include/args.h>
#include <include/nlohmann_json.hpp>
#include <include/tools_flags.h> // 标志
// 读图相关
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/opencv.hpp" // TODO 调试显示图片用，正式时不必要
// 编码转换
#include<codecvt>
std::wstring_convert<std::codecvt_utf8<wchar_t>> conv_Ustr_Wstr; // string utf-8 与 wstring utf-16 的双向转换器

using namespace std;
using namespace nlohmann;


namespace tool {
  // ============== 执行状态 ======================

  int ToolCode = 0; // 缓存本回合错误码
  string ToolMsg = ""; // 缓存本回合错误提示
  // 获取状态
  void get_state(int& code, string& msg) {
	code = ToolCode;
	msg = ToolMsg;
  }
  // 设置状态
  void set_state(int code = CODE_INIT, string msg = "") {
	ToolCode = code;
	ToolMsg = msg;
  }
  // 专门用于消息的wstring转string，转换失败时返回默认提示文字
  string msg_wstr_2_ustr(wstring& msg) {
	try {
	  string msgU8 = conv_Ustr_Wstr.to_bytes(msg); // 转回u8
	  return msgU8;
	}
	catch (...) {
	  return "wstring failed to convert to utf-8 string";
	}
  }

  // ================ 杂 ==========================

	// 退出程序前暂停
	void exit_pause(int x = 1) {
	  cout << "OCR exit." << endl; // 退出提示 
	  if (FLAGS_use_system_pause) system("pause");
	  exit(x);
	}
 
  // 多字节ANSI字符数组转宽字符数组
  wchar_t* char_2_wchar(char* c) {
	setlocale(LC_ALL, ""); // 程序的区域设置为windows系统当前区域
	size_t lenWchar = mbstowcs(NULL, c, 0); // 得到转为宽字符串的长度
	wchar_t* wc = new wchar_t[lenWchar + 1]; // 存放文件名的宽字符串
	int n = mbstowcs(wc, c, lenWchar + 1); // 多字节转宽字符
	setlocale(LC_ALL, "C"); // 还原区域设置为默认
	return wc;
  }

  // =============== 配置 ==============================

  // 加载一条json
  std::string load_json_str(string& str_in, bool& is_image, bool& is_hotupdate) {
	is_image = false;
	is_hotupdate = false;
	string logstr = "";
	try {
	  auto j = json::parse(str_in); // 转json对象 
	  for (auto& el : j.items()) { // 遍历键值 
		string value = to_string(el.value());
		int vallen = value.length();
		if (vallen > 2 && value[0] == '\"' && value[vallen - 1] == '\"') {
		  value = value.substr(1, vallen - 2); // 删去nlohmann字符串的两端引号
		}
		if (el.key() == "image_dir" || el.key() == "image_path") { // 图片路径
		  //str_in = utf8_2_gbk(value);
		  str_in = value; // 直接返回utf-8
		  is_image = true;
		}
		else { // 其它参数
		  // 设置一组配置。自动检验是否存在和合法性。优先级高于启动参数。
		  string getlog = google::SetCommandLineOption(el.key().c_str(), value.c_str());
		  int getlen = getlog.length();
		  if (getlen > 0 && (getlog[getlen - 1] == '\n' || getlog[getlen - 1] == '\r\n')) {
			getlog = getlog.substr(0, getlen - 1); // 删去SetCommandLineOption结尾换行
		  }
		  logstr += getlog + ". ";
		  is_hotupdate = true;
		}
	  }
	  if (!is_image) { // 转json成功，且json中没有图片路径 
		str_in = "";
	  }
	}
	catch (...) {
	  logstr = "[Error] Load json fail.";
	  is_hotupdate = true;
	}
	return logstr;
  }

  // 解析一行配置文本中的键和值，成功返回true。 
  bool get_line_config(string s, string& key, string& value) {
	int i = 0;
	// 去到第一个空格，截取第一段key
	while (s[i] != '\0' && s[i] != ' ')  ++i;
	if (s[i] == '\0' || i == 0) return false;
	key = s.substr(0, i);

	// 去到空格结束处，截取第二段value
	while (s[i] != '\0' && s[i] == ' ')  ++i;
	if (s[i] == '\0') return false;
	value = s.substr(i);
	return true;
  }

  // 从image_dir加载配置文件，参数写入FLAGS
  void load_congif_file() {
	// 提取已有Flags
	std::vector<google::CommandLineFlagInfo> allFlags;
	GetAllFlags(&allFlags);

	// 读入配置文件
	bool isDefaultDir = false;
	if (FLAGS_config_path == "") { // 未传入配置文件路径，则默认路径为 程序名+后缀 
	  string exe_name = google::ProgramInvocationShortName();
	  string::size_type dPos = exe_name.rfind('.'); // 最后一个点位置
	  FLAGS_config_path = exe_name.substr(0, dPos) + (string)"_config.txt";
	  isDefaultDir = true;
	}
	ifstream inConf(FLAGS_config_path);
	if (!inConf) { // 传入配置文件不存在时报错
	  if (!isDefaultDir) std::cerr << "[ERROR] config path not exist! config_dir: " << FLAGS_config_path << endl;
	  return;
	}

	// 解析配置文件
	string s;
	while (getline(inConf, s)) // 按行读入信息 
	{
	  if (s[0] == '#') continue; // 排除注释 
	  string key, value;
	  if (!get_line_config(s, key, value)) continue; // 解析一行配置 

	   // 设置一组配置。自动检验是否存在和合法性。优先级低于启动参数。
	  google::SetCommandLineOptionWithMode(key.c_str(), value.c_str(), google::SET_FLAG_IF_DEFAULT);
	}
	inConf.close();
  }

  // ================ 读图 =============================

  // 检查路径pathW是否为文件，是返回true
  bool is_exists_wstr(wstring pathW) {
	struct _stat buf;
	int result = _wstat((wchar_t*)pathW.c_str(), &buf);
	if (result != 0) { // 发生错误
	  return false;
	}
	if (S_IFREG & buf.st_mode) { // 是文件
	  return true;
	}
	// else if (S_IFDIR & buf.st_mode) { // 是目录
	   //return false;
	// }
	return false;
  }

  // 代替 cv::imread ，从路径pathW读入一张图片。pathW必须为unicode的wstring
  cv::Mat imread_wstr(wstring pathW, int flags = cv::IMREAD_COLOR) {
    string pathU8 = msg_wstr_2_ustr(pathW); // 再转回utf-8，以备输出错误。
    // ↑ 由于这个函数要被剪贴板CF_UNICODETEXT等复用，可能调用方只能提供wstring，所以多此一举转换一次。
    if (!is_exists_wstr(pathW)) { // 路径不存在
      set_state(CODE_ERR_PATH_EXIST, MSG_ERR_PATH_EXIST(pathU8)); // 报告状态：路径不存在且无法输出
      return cv::Mat();
    }
    FILE* fp = _wfopen((wchar_t*)pathW.c_str(), L"rb"); // wpath强制类型转换到whar_t，尝试打开文件
    if (!fp) { // 打开失败
      set_state(CODE_ERR_PATH_READ, MSG_ERR_PATH_READ(pathU8)); // 报告状态：无法读取
      return cv::Mat();
    }
    // 将文件读到内存
    fseek(fp, 0, SEEK_END); // 设置流 fp 的文件位置为 SEEK_END 文件的末尾
    long sz = ftell(fp); // 获取流 fp 的当前文件位置，即总大小（字节）
    char* buf = new char[sz]; // 存放文件字节内容
    fseek(fp, 0, SEEK_SET); // 设置流 fp 的文件位置为 SEEK_SET 文件的开头
    long n = fread(buf, 1, sz, fp); // 从给定流 fp 读取数据到 buf 所指向的数组中，返回成功读取的元素总数
    cv::_InputArray arr(buf, sz); // 转换为OpenCV数组
    cv::Mat img = cv::imdecode(arr, flags); // 解码内存数据，变成cv::Mat数据
    delete[] buf; // 释放buf空间
    fclose(fp); // 关闭文件
    if (!img.data) {
      set_state(CODE_ERR_PATH_DECODE, MSG_ERR_PATH_DECODE(pathU8)); // 报告状态：解码失败
    }
    return img;
  }

  // 从剪贴板读入一张图片。
  cv::Mat imread_clipboard(int flags = cv::IMREAD_COLOR) {
    // 参考文档： https://docs.microsoft.com/zh-cn/windows/win32/dataxchg/using-the-clipboard

    // 尝试打开剪贴板，锁定，防止其他应用程序修改剪贴板内容
    if (!OpenClipboard(NULL)) {
      set_state(CODE_ERR_CLIP_OPEN, MSG_ERR_CLIP_OPEN); // 报告状态：剪贴板打开失败
    }
    else {
      static UINT auPriorityList[] = {  // 允许读入的剪贴板格式：
        CF_UNICODETEXT,                 // unicode字符串
        CF_HDROP,                       // 文件列表句柄（文件管理器选中文件复制）
        CF_BITMAP,                      // 位图
      };
      int auPriorityLen = sizeof(auPriorityList) / sizeof(auPriorityList[0]); // 列表长度
      int uFormat = GetPriorityClipboardFormat(auPriorityList, auPriorityLen); // 获取当前剪贴板内容的格式
      // 根据格式分配不同任务。
      //     若任务成功，释放全部资源，关闭剪贴板，返回图片mat。
      //     若任务失败，释放已打开的资源和锁，报告状态，跳出switch，统一关闭剪贴板和返回空mat
      switch (uFormat)
      {
      case CF_UNICODETEXT: { // 1. unicode字符串 ==========================================================
        HANDLE hClip = GetClipboardData(uFormat); // 1.1. 以指定格式从剪贴板中检索数据，返回指定格式的剪贴板对象的句柄
        if (hClip) { // 获取成功
          wchar_t* pBuf = (wchar_t*)GlobalLock(hClip); // 1.2. 锁定全局内存对象，并返回指向该对象的内存块的第一个字节的指针
          wstring wpath = pBuf;
          // 释放资源
          GlobalUnlock(hClip); // 1.x.1. 释放内存对象，递减与GMEM_MOVEABLE一起分配的内存对象关联的锁定计数
          CloseClipboard(); // 1.x.2. 关闭剪贴板，使其他窗口能够继续访问剪贴板
          return imread_wstr(wpath); // 1.成功：尝试根据这个字符串读取文件
        }
        set_state(CODE_ERR_CLIP_DATA, MSG_ERR_CLIP_DATA); // 报告状态：获取剪贴板数据失败
        break;
      }

      case CF_HDROP: { // 2. 文件列表句柄 =========================================================== 
        HDROP hClip = (HDROP)GetClipboardData(uFormat); // 2.1. 得到文件列表的句柄
        if (hClip) {
          // https://docs.microsoft.com/zh-CN/windows/win32/api/shellapi/nf-shellapi-dragqueryfilea
          GlobalLock(hClip); // 2.2. 锁定全局内存对象
          int iFiles = DragQueryFile(hClip, 0xFFFFFFFF, NULL, 0); // 2.3. 0xFFFFFFFF表示返回文件列表的计数
          if (iFiles != 1) { // 只允许1个文件
            GlobalUnlock(hClip);
            set_state(CODE_ERR_CLIP_FILES, MSG_ERR_CLIP_FILES(iFiles)); // 报告状态：文件的数量不为1
            break;
          }
          //for (int i = 0; i < iFiles; i++) {
          int i = 0; // 只取第1个文件
          UINT lenChar = DragQueryFile(hClip, i, NULL, 0); // 2.4. 得到第i个文件名读入所需缓冲区的大小（字节）
          char* nameC = new char[lenChar + 1]; // 存放文件名的字节内容
          DragQueryFile(hClip, i, nameC, lenChar + 1); // 2.5. 读入第i个文件名
          wchar_t* nameW = char_2_wchar(nameC); // 2.6. 文件名转为宽字节数组
          cv::Mat mat = imread_wstr(nameW); // 2.7. 尝试读取文件
          // 释放资源
          delete[] nameC;
          delete[] nameW;
          GlobalUnlock(hClip); // 2.x.1 释放文件列表句柄
          CloseClipboard(); // 2.x.2 关闭剪贴板
          return mat;
        }
        set_state(CODE_ERR_CLIP_DATA, MSG_ERR_CLIP_DATA); // 报告状态：获取剪贴板数据失败
        break;
      }

      case CF_BITMAP: { // 3. 位图 ===================================================================
        HBITMAP hbm = (HBITMAP)GetClipboardData(uFormat); // 3.1. 从剪贴板中录入指针，得到文件句柄
        if (hbm) {
          // GlobalLock(hbm); // 返回值总是无效的，读位图似乎不需要锁？
        // https://social.msdn.microsoft.com/Forums/vstudio/en-US/d2a6aa71-68d7-4db0-8b1f-5d1920f9c4ce/globallock-and-dib-transform-into-hbitmap-issue?forum=vcgeneral
          BITMAP bmp; // 存放指向缓冲区的指针，缓冲区接收有关指定图形对象的信息
          GetObject(hbm, sizeof(BITMAP), &bmp); // 3.3. 获取图形对象的信息（不含图片内容本身）
          if (!hbm) {
            set_state(CODE_ERR_CLIP_GETOBJ, MSG_ERR_CLIP_GETOBJ); // 报告状态：检索图形对象信息失败
            break;
          }
          int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel / 8; // 根据色深计算通道数，32bit为4，24bit为3
          // 3.4. 将句柄hbm中的位图复制到缓冲区
          long sz = bmp.bmHeight * bmp.bmWidth * nChannels; // 图片大小（字节）
          cv::Mat mat(cv::Size(bmp.bmWidth, bmp.bmHeight), CV_MAKETYPE(CV_8U, nChannels));  // 创造空矩阵，传入位图大小和深度
          long getsz = GetBitmapBits(hbm, sz, mat.data); // 将句柄hbm中sz个字节复制到缓冲区img.data
          if (!getsz) {
            set_state(CODE_ERR_CLIP_BITMAP, MSG_ERR_CLIP_BITMAP); // 报告状态：获取位图数据失败
            break;
          }
          CloseClipboard();  // 释放资源
          // 3.5. 返回合适的通道
          if (mat.data) {
            if (nChannels == 1 || nChannels == 3) { // 1或3通道，PPOCR可识别，直接返回
              return mat;
            }
            else if (nChannels == 4) { // 4通道，PPOCR不可识别，删去alpha转3通道再返回
              cv::Mat mat_c3;
              cv::cvtColor(mat, mat_c3, cv::COLOR_BGRA2BGR); // 色彩空间转换
              return mat_c3;
            }
            set_state(CODE_ERR_CLIP_CHANNEL, MSG_ERR_CLIP_CHANNEL(nChannels)); // 报告状态：通道数异常
            break;
          }
          // 理论上上面 !getsz 已经 break 了，不会走到这里。保险起见再报告一次
          set_state(CODE_ERR_CLIP_BITMAP, MSG_ERR_CLIP_BITMAP); // 报告状态：获取位图数据失败
          break;
        }
        set_state(CODE_ERR_CLIP_DATA, MSG_ERR_CLIP_DATA); // 报告状态：获取剪贴板数据失败
        break;
      }

      case NULL: // 剪贴板为空
        set_state(CODE_ERR_CLIP_EMPTY, MSG_ERR_CLIP_EMPTY); // 报告状态：剪贴板为空
        break;
      case -1: // 其它不支持的格式
      default: // 未知
        set_state(CODE_ERR_CLIP_FORMAT, MSG_ERR_CLIP_FORMAT); // 报告状态： 剪贴板的格式不支持
        break;
      }
      CloseClipboard(); // 为break的情况关闭剪贴板，使其他窗口能够继续访问剪贴板。
    }
    return cv::Mat();
  }

  // ===== 每回合的入口：读取图片 =============
  // 代替 cv::imread ，从路径pathU8读入一张图片。pathU8必须为utf-8的string
  cv::Mat imread_utf8(string pathU8, int flags = cv::IMREAD_COLOR) {
    set_state(); // 报告状态：初始化
    if (pathU8 == u8"clipboard") { // 若为剪贴板任务
      return imread_clipboard(flags);
    }
    // string u8 转 wchar_t
    std::wstring wpath;
    try {
      wpath = conv_Ustr_Wstr.from_bytes(pathU8); // 利用转换器转换
    }
    catch (...) {
      set_state(CODE_ERR_PATH_CONV, MSG_ERR_PATH_CONV(pathU8)); // 报告状态：转wstring失败
      return cv::Mat();
    }
    return imread_wstr(wpath);
  }
}


/* 备份
* 

D:\图片\Screenshots\t1.png

--image_dir=clipboard

  // 检查路径pathU8，若存在，返回true
  bool is_exists(string pathU8) {
	cout << u8"开始检查： " << pathU8 << endl;
	struct _stat buf;
	int result = _stat(pathU8.c_str(), &buf);
	if (result == 0) {
	  cout << u8"检查0： " << pathU8 << endl;
	  return false;
	}
	if (S_IFREG & buf.st_mode) {
	  cout << u8"检查为文件： " << pathU8 << endl;
	  return true;
	}
	else if (S_IFDIR & buf.st_mode) {
	  cout << u8"检查为目录： " << pathU8 << endl;
	  return false;
	}
	cout << u8"检查失败： " << pathU8 << endl;
  }


cv::Mat readByImread(string path) {
  return cv::imread(path, cv::IMREAD_COLOR);
}

cv::Mat readByImdecode(string path) {
  std::ifstream imgStream(path.c_str(), std::ios::binary);
  imgStream.seekg(0, std::ios::end);
  size_t fileSize = imgStream.tellg();
  imgStream.seekg(0, std::ios::beg);
  if (fileSize == 0) {
	return cv::Mat();
  }
  std::vector<unsigned char> data(fileSize);
  imgStream.read(reinterpret_cast<char*>(&data[0]), sizeof(unsigned char) * fileSize);
  if (!imgStream) {
	return cv::Mat();
  }
  return cv::imdecode(cv::Mat(data), cv::IMREAD_COLOR);
}

  // 判断字符串是否utf-8
  bool is_utf8(const string& str)
  {
	char nBytes = 0; // UFT8可用1-6个字节编码,ASCII用一个字节
	unsigned char chr;
	bool bAllAscii = true; // 如果全部都是ASCII, 说明不是UTF-8
	for (int i = 0; i < str.length(); i++)
	{
	  chr = str[i];
	  // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,
	  // 但用一个字节存,最高位标记为0,o0xxxxxxx
	  if ((chr & 0x80) != 0)
		bAllAscii = false;
	  // 如果不是ASCII码,应该是多字节符,计算字节数
	  if (nBytes == 0)
	  {
		if (chr >= 0x80)
		{
		  if (chr >= 0xFC && chr <= 0xFD)   nBytes = 6;
		  else if (chr >= 0xF8)         nBytes = 5;
		  else if (chr >= 0xF0)         nBytes = 4;
		  else if (chr >= 0xE0)         nBytes = 3;
		  else if (chr >= 0xC0)         nBytes = 2;
		  else {
			return false;
		  }
		  nBytes--;
		}
	  }
	  else //多字节符的非首字节,应为 10xxxxxx
	  {
		if ((chr & 0xC0) != 0x80)
		  return false;
		nBytes--;
	  }
	}

	if (nBytes > 0) //违返规则
	  return false;

	if (bAllAscii) //如果全部都是ASCII, 说明不是UTF-8
	  return false;

	return true;
  }
  // 判断字符串是否bgk
  bool is_gbk(const string& str)
  {
	unsigned int nBytes = 0;//GBK可用1-2个字节编码,中文两个 ,英文一个
	unsigned char chr = str.at(0);
	bool bAllAscii = true; //如果全部都是ASCII,

	for (unsigned int i = 0; str[i] != '\0'; ++i) {
	  chr = str.at(i);
	  if ((chr & 0x80) != 0 && nBytes == 0) {// 判断是否ASCII编码,如果不是,说明有可能是GBK
		bAllAscii = false;
	  }

	  if (nBytes == 0) {
		if (chr >= 0x80) {
		  if (chr >= 0x81 && chr <= 0xFE) {
			nBytes = +2;
		  }
		  else {
			return false;
		  }
		  nBytes--;
		}
	  }
	  else {
		if (chr < 0x40 || chr>0xFE) {
		  return false;
		}
		nBytes--;
	  }//else end
	}

	if (nBytes != 0) { // 违反规则
	  return false;
	}

	if (bAllAscii) { // 如果全部都是ASCII, 也是GBK
	  return true;
	}

	return true;
  }
  // 字符串gbk转utf-8
  string gbk_2_utf8(const string& str_gbk)
  {
	if (str_gbk == "" || is_utf8(str_gbk) || !is_gbk(str_gbk)) {
	  return str_gbk; // 已经是utf-8或者非gbk，不转
	}
	string str_utf8 = "";
	WCHAR* str1;
	int n = MultiByteToWideChar(CP_ACP, 0, str_gbk.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, str_gbk.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char* str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	str_utf8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return str_utf8;
  }
  // 字符串utf-8转gbk
  string utf8_2_gbk(const string& str_utf8) {
	if (str_utf8 == "")
	  return str_utf8;
	const char* strUTF8 = str_utf8.c_str();
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
  }

*/