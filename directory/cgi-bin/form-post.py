#!/usr/bin/python

# Import modules for CGI handling 
import cgi, cgitb 

# Create instance of FieldStorage 
form = cgi.FieldStorage() 

# Get data from fields
intra_id = form.getvalue('intra_id')
coalition = form.getvalue('coalition')

# Make response data

print ("Content-type:text/html\r\n\r\n")
print ("<html>")
print ("<head>")
print ("<title>Test Form (POST) Result</title>")
print ("<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">")
print ("<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>")
print ("<link href=\"https://fonts.googleapis.com/css2?family=Fredoka+One&display=swap\" rel=\"stylesheet\">")
print ("<style>")
print ("body {font-family: 'Fredoka One';}")
print ("</style>")
print ("</head>")
print ("<body>")
print ("<h2>Test Form (POST) Result </h2>")
print ("<h3>%s's coalition is %s</h3>" %(intra_id, coalition))
print("<a href=\"/form-test.html\"> * go to form test</a>")
print ("</body>")
print ("</html>")