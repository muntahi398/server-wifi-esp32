# server-wifi-esp32-continuous-communication

in the main folder, simple_wifi.cpp is the file for server part.

in the client part,, in a new terminal use "netcat ip socket"  like netcat 10.42.0.181  3000
this if connected successfully, shows 

Hello, congrats, server conn is on, send data 
 <
-- type value like 9 or any from 0 to 9, response of ringlight controller will appear.
--to end connection, type second inputx, like 5x. this will show goodbye and end socket connection.

