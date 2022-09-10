#include <xsystem/xsystem.h>
#include <string>
#include <sstream>
#include <stdio.h>
/********************************************************************************
* @File x_shell.cpp
* @Author: Freet-Bash
* @Date: 2022-09-10  11:44:25
* @Description: 跨平台shell popen调用
********************************************************************************/
std::string x_shell(std::string cmd){

	#ifdef _WIN32
			FILE *fp;
			char line[1024];
			if ((fp = _popen(cmd.c_str(), "r")) == NULL) {
				return "";
			}
			std::ostringstream stm;
			while (fgets(line, 64, fp)) {
				stm << line;
			}
			_pclose(fp);
			return stm.str();
	#elif __linux__
		FILE *fp;
		if (fp = popen(cmd.c_str(), "r")) {
			std::ostringstream stm;
			char line[1024];
			while (fgets(line, 64, fp)) {
				stm << line;
			}
			pclose(fp);
			return stm.str();
		}
	#endif
		return "";
	}