#ifndef _tools_
#define _tools_

string gbk_2_utf8(const string&);
string utf8_2_gbk(const string&);
string load_json_str(string&,bool&,bool&);
void load_congif_file();
void exit_pause(int x=1);

#endif