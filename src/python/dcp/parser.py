import collections, dcp
from PyQt4 import QtCore

class ParserResult(object):
    def __init__(self, msg):
        self._msg = msg
    @property
    def msg(self):
        return self._msg
    @property
    def snr(self):
        return int(self._msg.snr)
    @property
    def source(self):
        return bytes(self._msg.source)
    @property
    def destination(self):
        return bytes(self._msg.destination)
    @property
    def data(self):
        return bytes(self._msg.data)
    @property
    def flags(self):
        return int(self._msg.flags)
    @property
    def dcpFlags(self):
        return int(self._msg.dcpFlags)
    @property
    def userFlags(self):
        return int(self._msg.userFlags)

class Ack(ParserResult):
    def __init__(self, msg, error):
        super(Ack, self).__init__(msg)
        self._error = error
    def __repr__(self):
        return "Ack(%d, '%s', error=%d)" % (
            self.snr, self.source, self._error)
    @property
    def error(self):
        return self._error

class Reply(ParserResult):
    def __init__(self, msg, error, args):
        super(Reply, self).__init__(msg)
        self._error = error
        self._args = args
    def __repr__(self):
        return "Reply(%d, '%s', error=%d, args=%s)" % (
            self.snr, self.source, self._error, repr(self._args))
    @property
    def error(self):
        return self._error
    @property
    def args(self):
        return self._args

class Command(ParserResult):
    def __init__(self, msg, cmd, ident, args):
        super(Command, self).__init__(msg)
        self._cmd = cmd
        self._id = ident
        self._args = args
    def __repr__(self):
        return "Command(%d, '%s', cmd='%s', id='%s', args=%s)" % (
            self.snr, self.source, self._cmd, self._id, repr(self._args))
    @property
    def cmd(self):
        return self._cmd
    @property
    def id(self):
        return self._id
    @property
    def args(self):
        return self._args

class Parser(object):
    def __init__(self):
        self._replyParser = dcp.ReplyParser()
        self._commandParser = dcp.CommandParser()
    def __call__(self, msgs):
        if not isinstance(msgs, collections.Iterable):
            msgs = [msgs]
        result = []
        for m in msgs:
            r = None
            if m == None:
                pass
            elif m.isReply():
                p = self._replyParser
                if p.parse(m):
                    if p.isAckReply():
                        r = Ack(m, p.errorCode())
                    else:
                        byteargs = [bytes(a) for a in p.arguments()]
                        r = Reply(m, p.errorCode(), byteargs)
            elif not m.isNull():
                p = self._commandParser
                if p.parse(m):
                    bargs = [bytes(a) for a in p.arguments()]
                    r = Command(m, bytes(p.command()), bytes(p.identifier()),
                                byteargs)
            result.append(r)
        if len(result) == 0:
            result = None
        elif len(result) == 1:
            result = result[0]
        return result

def isValid(obj):
    isIterable = isinstance(obj, collections.Iterable)
    if not isIterable:
        obj = [obj]
    res = []
    for o in obj:
        if isinstance(o, Command):
            res.append(True)
        elif isinstance(o, Reply) and o.error <= 0:
            res.append(True)
        elif isinstance(o, Ack) and o.error == 0:
            res.append(True)
        else:
            res.append(False)
    if len(res) == 1 and (not isIterable):
        return res[0]
    else:
        return res
