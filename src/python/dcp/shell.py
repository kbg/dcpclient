from dcp import *
from dcp.parser import *
from dcp.dcpshell import DcpShell
from time import sleep

_shell = DcpShell()
connect = _shell.connect
disconnect = _shell.disconnect
isConnected = _shell.isConnected
send = _shell.send
ack = _shell.ack
reply = _shell.reply
wait = _shell.wait

def timeout():
    return _shell.timeout

def setTimeout(msecs):
    _shell.timeout = msecs

def purgeTimeout():
    return _shell.purgeTimeout

def setPurgeTimeout(msecs):
    _shell.purgeTimeout = msecs
