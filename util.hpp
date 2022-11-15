#include <iostream>
#include <string>
#include <vector>
#include <cstring>
// #include <algorithm>

namespace util
{
	void remove_last_semicolon(std::string &line)
	{
		if (line.back() == ';')
			line.pop_back();
	}

	std::vector<std::string> ft_split(std::string s, std::string divid) {
		std::vector<std::string> v;
		char* c = strtok((char*)s.c_str(), divid.c_str());
		while (c) {
			v.push_back(c);
			c = strtok(NULL, divid.c_str());
		}
		return v;
	}
}