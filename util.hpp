#ifndef UTIL_CLASS_HPP
# define UTIL_CLASS_HPP
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <time.h>
#include <sstream>
// #include <algorithm>

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
}

#endif
