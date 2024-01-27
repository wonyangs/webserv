#!/usr/bin/python3

import cgi
import sys
import os
# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
first_name = form.getvalue('firstname')
last_name = form.getvalue('lastname')
# first_name = "wonyang"
# last_name = "wonseok"
# print("HTTP/1.1 200 OK")
print("Content-type: text/html\r\n\r\n")
print(os.environ["QUERY_STRING"])
print("<html>")
print("<head>")
print("<title>Hello - Second CGI Program</title>")
print("<html>")
print("<head>")
print("<h2>Hello %s %s</h2>" % (first_name, last_name))
print("</body>")
print("</html>")