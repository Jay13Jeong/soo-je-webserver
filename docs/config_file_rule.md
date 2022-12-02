# Config 파일 작성 규칙

## 일반 규칙

- 대괄호(`{}`) `server` 블록과 `location` 블록을 만드는 곳에서만 그 의미를 갖는다.
- 대괄호는 모두 쌍을 맞춰야 한다.
- `location` 블록은 `server` 블록 내에서도 가장 마지막에 있는 것이 좋다.
- `server` 블록과 `location` 블록 시작 부분, 닫는 괄호, 주석을 제외한 줄은 모두 세미콜론(`;`) 1개로 끝나야 한다.
- `#` 로 시작하는 한 줄은 주석으로 처리한다.

## 키워드에 관한 규칙 (일반)

사용할 수 있는 키워드들

- `server`
- `listen`
- `host` : 값으로 `"127.0.0.1"`만 갖는다.
- `server_name`
- `root`
- `index`
- `default_error_pages`
	- `<키워드> <상태 코드> <에러 페이지>` 형식
- `autoindex`
- `client_max_body_size`
- `cgi`
	- `<키워드> <확장자> <프로그램>` 형식
- `location`
	- `<키워드> <path> {` 형식

이 외의 키워드가 있는 경우 → Error

## 키워드에 관한 규칙 (location)

`location` 블록 내에 올 수 있는 키워드들

- `root`
- `autoindex`
- `client_max_body_size`
- `return`
	- `<키워드> <상태코드> <목적지>` 형식
- `index`
- `accept_method`
- `cgi`
	- `<키워드> <확장자> <프로그램>` 형식

이 외의 키워드가 오는 경우 → Error

자세한 규칙 → [Notion](https://www.notion.so/Config-dbad5cdd8cb9489ab64b8575fa2679a9)
