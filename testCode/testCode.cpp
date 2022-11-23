#include<iostream>
#include <string>
#include <vector>
#include <cstring>

struct s_split {
public:
	std::vector<std::string> ft_split(std::string s, std::string divid) {
		std::vector<std::string> v;
		char *c = strtok((char*)s.c_str(), divid.c_str());
		while (c) {
			v.push_back(c);
			c = strtok(NULL, divid.c_str());
		}
		return v;
	}
};

int main(){
    std::string test = "   	123{ 	 		123	4 	  123	456   }	 	";
    std::vector<std::string> result = ft_split(test, "\t ");
    for (int i=0; i < result.size(); i++){
        std::cerr << i << ':' << result[i] << std::endl;
    }
}