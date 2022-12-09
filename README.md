# Handmade webserver "Soo-Je"
### Handmade webserver made of CPP.<br>
### This webserver was created by referring to NGINX.

<hr>

![webserver drawio](https://user-images.githubusercontent.com/63899204/201680135-0c19da6d-e505-4baa-8ee8-dbad0956de8a.png)


## Usage

This program can receive a single arg, and if there is no arg, it runs "default.conf" by default.<br>
The setup of the conf file is based on nginx.<br>
Then connect to the browser with the open port printed on the terminal (Enter "localhost:8011")<br>

## Conf file setting
<b>listen  :</b> Specify a port.<br>
<b>host    :</b> Specify a host.<br>
<b>server_name :</b> Specify server name.<br>
<b>root    :</b> Directory on the server that connects to URI.<br>
<b>index   :</b> Default files in the corresponding URI directory.<br>
<b>cgi     :</b> The extension of the cgi to use and the location of the launcher. (ex: cgi .py /usr/bin/python3)<br>
<b>default_error_pages :</b> override the error page to display if an error occurs.<br>
<b>location :</b> override the behavior of the server based on the path of URI.<br>
<b>accept_method :</b> Only allowed methods can be handled by the server with the current path.<br>
<b>autoindex :</b> If this option is "on", the 404 page will not pop up, and the list of files in the destination folder will be listed.<br>
<b>client_max_body_size :</b> Maximum acceptable payload size.<br>
<b>return :</b> 301 permanently moved.<br>

## Browser Support
This server has been tested in a Chrome environment.<br>

## Reference
https://www.freebsd.org/cgi/man.cgi?kqueue
<br>
https://www.rfc-editor.org/rfc/rfc7230
<br>
https://www.rfc-editor.org/rfc/rfc7231

## Scenes tested with server tester (siege -b test, parrot test)

![testtt](https://user-images.githubusercontent.com/63899204/206649410-06704daf-8e62-455a-b545-33cd06d329f8.gif)

![a2](https://user-images.githubusercontent.com/63899204/205376751-c971d989-a67c-4b06-9ea1-ac6e6241b247.gif)
