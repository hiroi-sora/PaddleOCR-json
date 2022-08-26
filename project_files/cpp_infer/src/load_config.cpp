
#include<fstream>
#include <iostream>
#include <filesystem>

#include <include/args.h>
using namespace std;

bool get_line_config(string s,string &key, string &value) { // 解析一行文本中的键和值，成功返回true。 
  int i = 0;
  // 去到第一个空格，截取第一段key
  while (s[i] != '\0' && s[i] != ' ')  ++i; 
  if (s[i] == '\0' || i==0) return false;
  key = s.substr(0, i);

  // 去到空格结束处，截取第二段value
  while (s[i] != '\0' && s[i] == ' ')  ++i;
  if (s[i] == '\0') return false;
  value = s.substr(i);
  return true;
}

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
    if(!isDefaultDir) std::cerr << "[ERROR] config path not exist! config_dir: " << FLAGS_config_path << endl;
    return;
  }

  // 解析配置文件
  string s;
  while (getline(inConf, s)) // 按行读入信息 
  {
    if (s[0] == '#') continue; // 排除注释 
    string key, value;
    if ( !get_line_config(s, key, value) ) continue; // 解析一行配置 

     // 设置一组配置。自动检验是否存在和合法性。优先级低于启动参数。
     google::SetCommandLineOptionWithMode(key.c_str(), value.c_str(), google::SET_FLAG_IF_DEFAULT);
  }
  inConf.close();
}

//if ( find_if(allFlags.begin(), allFlags.end(), [&](auto v) {return v.name == key;} ) == allFlags.end())
//  continue;  // 排除配置文件中不合法（不属于Flags内）的key
//if ( !google::GetCommandLineFlagInfoOrDie(key.c_str()).is_default )
//  continue;  // 排除启动参数中存在的flags（启动参数优先级高于配置文件） 
 //cout << key << " : " << value << endl;