#!/usr/bin/env python

import socket


def SendCommand(socket,cmd):
    newcmd ="{:<50}".format(cmd)
    print "Sending ",newcmd
    s.send(newcmd)


TCP_IP = '192.168.1.10'
TCP_PORT = 2705
BUFFER_SIZE = 50

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))

data = s.recv(BUFFER_SIZE)
print data


SendCommand(s,"configure")
data = s.recv(BUFFER_SIZE)
print "received data:%s"%(data)

SendCommand(s,"exit")


#a= raw_input()
