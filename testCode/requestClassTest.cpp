#include <iostream>
#include "../Request.hpp"
#include "../util.hpp"
    //map에서의 iterator를 사용한 요소 접근의 경우 iter->first, iter->second와 같은 형식으로
    //사용할 수 있다.

void	testFunc(std::string & data, std::string &status, std::string resultStatus)
{
	Request request;

	request.parse(data, status);
	if (status == resultStatus)
	{
		std::cerr << "test OK!! " << std::endl;
		return ;
	}
	else
		std::cerr << "errer!!  status : resultStatus = " << status << " : " << resultStatus << std::endl << std::endl;

	std::cerr << "Method : " << request.getMethod() << std::endl;
	std::cerr << "Target : " << request.getTarget() << std::endl;
	std::cerr << "Version : " << request.getVersion() << std::endl;
	std::cerr << "Headers: " << std::endl;
	std::map<std::string, std::string> temp = request.getHeaders();
	std::map<std::string,std::string>::iterator iter;
	for(iter = temp.begin() ; iter != temp.end(); iter++){
		std::cerr << iter->first << " : "<< iter->second << std::endl;
	}
	std::cerr << "Body : " << request.getBody() << std::endl;

}

int main(void)
{
	std::string data;
	std::string status;

	data = "GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9";
	std::cerr << "1---------------------------------------------------" << std::endl;
	testFunc(data, status, "200"); //기본 GET 성공
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "DELETE / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9";
	std::cerr << "2---------------------------------------------------" << std::endl;
	testFunc(data, status, "200"); //기본 DELETE 성공
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "PUT / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9";
	std::cerr << "3---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");// 지원하지 않는 메소드 PUT// 지금은 지원함
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9\r\n";
	std::cerr << "4---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");//마지막 헤더 문자열에 \r\n추가
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9\r\n\r\n";
	std::cerr << "5---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");//마지막에 \r\n 만 있는 문자열 추가
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "GET / HTTP/1.0\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9";
	std::cerr << "6---------------------------------------------------" << std::endl;
	testFunc(data, status, "505");//지원하지 않는 HTTP 버전
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "GET / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9\r\n\r\n123";
	std::cerr << "7---------------------------------------------------" << std::endl;
	testFunc(data, status, "400");//get 메소드에 바디 추가
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9\r\n123";
	std::cerr << "8---------------------------------------------------" << std::endl;
	testFunc(data, status, "400");//POST 메소드에 헤더 바디사이에 빈 문자열 없이 바디 추가
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9\r\n\r\n123\r\n456";
	std::cerr << "9---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");//POST 메소드에 바디 개행이 \r\n 이 경우 설정
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9\r\n\r\n123\n456";
	std::cerr << "10---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");//POST 메소드에 바디 개행이 \n 이 경우 설정
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nAccept-Encoding: gzip, deflate, br\r\nTransfer-Encoding: chunked, gzip\r\n\r\n";
	std::cerr << "11---------------------------------------------------" << std::endl;
	testFunc(data, status, "800");//POST 청크 확인. 바디 없음
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nAccept-Encoding: gzip, deflate, br\r\nTransfer-Encoding: gzip\r\n\r\n";
	std::cerr << "12---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");//POST 청크 확인. 바디 없음, 헤더에 청크값 없음
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nAccept-Encoding: gzip, deflate, br\r\nTransfer-Encoding: chunked, gzip\r\n\r\n1\r\n1\r\n0\r\n\r\n";
	std::cerr << "13---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");//POST 청크 확인. 바디 1줄 있음
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nAccept-Encoding: gzip, deflate, br\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n";
	std::cerr << "14---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");//POST 청크 확인. 바디 없이 바로 종료청크 올때
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nAccept-Encoding: gzip, deflate, br\r\nTransfer-Encoding: chunked, gzip\r\n\r\n1\r\n1\r\n9\r\n1q2q3q\n4q\r\n0\r\n\r\n";
	std::cerr << "15---------------------------------------------------" << std::endl;
	testFunc(data, status, "200");//POST 청크 확인. 바디 있음 여러줄,, 자음이 오면 하나당 길이 2
	std::cerr << "---------------------------------------------------" << std::endl;

	data = "POST / HTTP/1.1\r\nHost: localhost:8080\r\nConnection: keep-alive\r\nAccept-Encoding: gzip, deflate, br\r\nTransfer-Encoding: chunked\r\n\r\n1\r\n1\r\n8\r\n1q2w3e4\r\n0\r\n\r\n";
	std::cerr << "16---------------------------------------------------" << std::endl;
	testFunc(data, status, "400");//POST 청크 확인.청크길이와 문자열 길이가 다른 경우
	std::cerr << "---------------------------------------------------" << std::endl;
	return (0);
}