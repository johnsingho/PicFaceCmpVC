@echo off

set dstpc=192.168.6.72
set sharedir=newSDK
set uname=administrator
set upass=cat

net use z: \\%dstpc%\%sharedir% "%upass%" /user:"%uname%" /persistent:no


