#!/usr/bin/python3

import os

print ("Set-Cookie: my_CGI=hello tom!; Expires=Wed, 21 Oct 2023 07:28:00 GMT\r\n", end ="")
print ("Content-type:text/html\r\n\r\n")

cookie = str(os.environ.get('HTTP_COOKIE'))
if cookie != "None":
    print("My cookie is ", cookie)
else:
    print("No cookie....")
print("<br>")
print ("i am python^.^")
print("<br>")
print('<a href="/"><h3>BACK TO THE HOME</h3></a>')
for key, value in os.environ.items():
    print('{}: {}<br>'.format(key, value))
