#!/usr/bin/env python

import socket


def SendCommand(socket,cmd):
    newcmd ="{:<1024}".format(cmd)
    print "Sending ",newcmd
    s.send(newcmd)

def nullstrip(s):
    """Return a string truncated at the first null character."""
    try:
        s = s[:s.index('\x00')]
    except ValueError:  # No nulls were found, which is okay.
        pass
    return s

TCP_IP = '192.168.0.12'
TCP_PORT = 2705
BUFFER_SIZE = 1024



print "####################################################"
print "#       Welcome to ATLASPix PEARY server !!        #"
print "#                                                  #"
print "# Commands available :                             #"
print "#      configure                                   #"
print "#      start_run                                   #"
print "#      stop_run                                    #"
print "#      setThreshold Th(V)                          #"
print "#      setRegister name value                      #"
print "#      setDataFileName path/filename               #"
print "#      LoadConfig path/basename                    #"
print "#      WriteConfig path/basename                   #"
print "#      exit                                        #"
print "####################################################"

print ""
print "Setting up connection to %s:%s"%(TCP_IP,TCP_PORT)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
data = s.recv(BUFFER_SIZE)
print "%s" % nullstrip(data)

cmd=""

while(cmd!="exit"):
    cmd = raw_input("peary> ")
    SendCommand(s,cmd)
    data = s.recv(BUFFER_SIZE)
    print "%s"%(nullstrip(data))


