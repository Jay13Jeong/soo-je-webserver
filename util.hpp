#ifndef UTIL_CLASS_HPP
# define UTIL_CLASS_HPP
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <time.h>
#include <sstream>
#include <dirent.h> //DT_DIR
#include <sys/stat.h>//mkdir
#include <sys/types.h>//mkdir
#include <unistd.h>//unlink, rmdir
#include <algorithm>
#include <cctype>

namespace util
{
	bool remove_last_semicolon(std::string &line)
	{
		if (line.back() == ';')
		{
			line.pop_back();
			return (true);
		}
		return (false);
	}

	std::vector<std::string> ft_split(std::string s, std::string divid) {
		std::vector<std::string> v;
		if (s.length() == 0)
			return v;
		char* c = strtok((char*)s.c_str(), divid.c_str());
		while (c) {
			v.push_back(c);
			c = strtok(NULL, divid.c_str());
		}
		return v;
	}

	std::vector<std::string> ft_split_s(std::string s, std::string divid)
	{
		std::vector<std::string> v;
		size_t pos;
		if (s.length() == 0)
			return v;
		pos = s.find(divid);
		while (pos != std::string::npos)
		{
			v.push_back(s.substr(0, pos));
			s = s.substr(pos + divid.length(), s.length() - v.back().length() + divid.length());
			pos = s.find(divid);
		}
		if (s.length() != 0)
			v.push_back(s);
		return v;
	}

	bool is_numeric(std::string str)
	{
		char* p;
		strtol(str.c_str(), &p, 10);
		return *p == 0;
	}

	int count_semicolon(std::string str)
	{
		int ret = 0;
		for (int i = 0; i < str.length(); i++)
		{
			if (str[i] == ';')
				ret++;
		}
		return (ret);
	}

	int count_sp(std::string str)
	{
		int ret = 0;
		for (int i = 0; i < str.length(); i++)
		{
			if (str[i] == ' ')
				ret++;
		}
		return (ret);
	}

	template <typename T>
	T string_to_num(std::string str)
	{
		std::stringstream ss(str);
		T ret;
		ss >> ret;
		return (ret);
	}

	template <typename T>
	std::string num_to_string (T num)
	{
		std::stringstream ss;
		ss << num;
		std::string ret = ss.str();
		return (ret);
	}

	std::string get_date()
	{
		// Thu, 17 Nov 2022 09:26:07 GMT (UTC == GMT)
		time_t raw_time;
		struct tm *time_info;
		time(&raw_time);
		time_info = gmtime(&raw_time); // UTC 형식 시간
		char buf[64];
		strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT", time_info);
		std::string ret = buf;
		return (ret);
	}

	//하위 폴더 및 파일을 모두 제거하는 함수.
	bool rm_sub_files(std::string path)
	{
		DIR *curr_dir; //현재 pwd위치.
		struct dirent *target; //현재 디렉토리의 구성 파일 또는 디렉토리.

		if ((curr_dir = opendir(path.c_str())) == NULL) //디렉토리 열기.
			return (false);
		if (path[path.length() - 1] != '/') //str.back()은 c++11이다...
			path += '/';
		while ((target = readdir(curr_dir)) != NULL) //파일리스트를 하나씩 읽기.(read할때마다 자동으로 넘어감).
		{
			std::string file_name(target->d_name);
			if (file_name == "." || file_name == "..")
				continue ;
			if (target->d_type == DT_DIR) //파일 타입이 디렉토리면...
			{
				if (rm_sub_files(path + file_name) == false) //디렉토리 비우기 및 삭제. (재귀로 하위폴더 처리).
					return (false);
			}
			else //파일이 정규파일이면 그냥 삭제.
				unlink((path + file_name).c_str());
		}
		rmdir(path.c_str()); //비워진 현재 디렉토리는 지운다.
		return (true);
	}

	//중간 디렉토리 경로를 만들어주는 메소드.
	bool make_middle_pathes(std::string path)
    {
        size_t n = 0;
        size_t pos = path.find("/", n); //경로의 첫번째 디렉토리를 지정.
        while (pos != std::string::npos) //디렉토리가 더이상 없다면 종료.
        {
            std::string temp = path.substr(0, pos); //찾은 단계까지의 총 경로.
            mkdir(temp.c_str(), 0755); //디렉토리 생성
            n = pos + 1; //문자하나씩 읽기.
            pos = path.find("/", n); //다음 폴더가 있는지 찾는다.
        }
        return true; //성공적으로 생성되었으면 true반환.
    }

	// string을 모두 소문자로 만들어주는 함수
	std::string to_lower_string(std::string str)
	{
		std::string ret = std::string(str.length(), '\0');
		std::transform(str.begin(), str.end(), ret.begin(), ::tolower);
		return (ret);
	}
}

#endif
