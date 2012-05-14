import collections, dcp, dcp.parser
from PyQt4 import QtCore

class DcpShell(object):
    def __init__(self):
        self._app = None
        self._timeout = 3000
        self._purgeTimeout = 60000
        self._client = dcp.Client()
        self._parse = dcp.parser.Parser()
        self._inQueue = collections.OrderedDict()
        QtCore.QObject.connect(self._client,
            QtCore.SIGNAL("error(Dcp::Client::Error)"), self._onClientError)
        QtCore.QObject.connect(self._client,
            QtCore.SIGNAL("stateChanged(Dcp::Client::State)"),
            self._onClientStateChanged)

    def _onClientError(self, error):
        """Client error slot."""
        if error != dcp.Client.SocketTimeoutError:
            print('Error: %s.' % self._client.errorString())

    def _onClientStateChanged(self, state):
        #print('State: %d.' % state)
        pass

    @property
    def client(self):
        """The DCP client instance."""
        return self._client

    @property
    def timeout(self):
        """Timeout value for blocking network operations in seconds."""
        return self._timeout / 1000.0

    @timeout.setter
    def timeout(self, value):
        self._timeout = int(round(value * 1000.0))

    @property
    def purgeTimeout(self):
        """Time in seconds until received messages are removed from the
        input queue. Set to -1 to disable purging."""
        return self._purgeTimeout / 1000.0

    @purgeTimeout.setter
    def purgeTimeout(self, value):
        self._purgeTimeout = int(round(value * 1000.0))

    def connect(self, server, port, device):
        """Connect to a DCP server.

        Parameters
        ----------
        server : string
            Host name of the DCP server (e.g. "gcs.tt.iac.es").
        port : int
            Port number the DCP server is listening to (e.g. 2001).

        Returns
        -------
        result : boolean
            True if a connection has been established; or False otherwise.

        See Also
        --------
        disconnect : Disconnect from DCP server.
        isConnected : Check if the client is connected to a DCP server.
        timeout : Set/get the timeout value for blocking network operations.
        """
        self._app = QtCore.QCoreApplication.instance()
        if self._app == None:
            self._app = QtCore.QCoreApplication([])

        if self._client.isConnected():
            self._client.disconnectFromServer()
            self._client.waitForDisconnected(self._timeout)

        self._client.connectToServer(server, port, device)
        if not self._client.waitForConnected(self._timeout):
            return False
        return self.isConnected()

    def disconnect(self):
        """Disconnect from DCP server.

        Returns
        -------
        result : boolean
            True if the connection has been closed; or False otherwise.

        See Also
        --------
        connect : Connect to a DCP server.
        isConnected : Check if the client is connected to a DCP server.
        timeout : Set/get the timeout value for blocking network operations.
        """
        self._client.disconnectFromServer()
        return self._client.waitForDisconnected(self._timeout)

    def isConnected(self):
        """Check if the client is connected to a DCP server.

        Returns
        -------
        result : boolean
            True if the client is connected to a DCP server; or False
            otherwise.
        """
        self._app.processEvents()
        return self._client.isConnected()

    def send(self, device, data):
        msg = self._client.sendMessage(device, data)
        self._client.waitForMessagesWritten(self._timeout)
        return msg

    def ack(self, msg, error=0):
        if isinstance(msg, dcp.parser.ParserResult):
            msg = msg.msg
        if msg.isReply():
            return None
        ackMsg = msg.ackMessage(error)
        self._client.sendMessage(ackMsg)
        self._client.waitForMessagesWritten(self._timeout)
        return ackMsg

    def reply(self, msg, data='', error=0):
        if isinstance(msg, dcp.parser.ParserResult):
            msg = msg.msg
        if msg.isReply():
            return None
        replyMsg = msg.replyMessage(data, error)
        self._client.sendMessage(replyMsg)
        self._client.waitForMessagesWritten(self._timeout)
        return replyMsg

    def _parseAvailableMessages(self):
        while self._client.messagesAvailable() > 0:
            msg = self._client.readMessage()
            if msg.isNull():
                continue
            res = self._parse(msg)

            # key tuple: (isReply, snr, source)
            # reply tuple: (t, Ack, Reply)
            # command tuple: (t, Command)
            isReply = msg.isReply()
            snr = int(msg.snr)
            source = bytes(msg.source)
            key = (isReply, snr, source)
            t = QtCore.QElapsedTimer(); t.start()

            if isReply:
                entry = self._inQueue.get(key)
                if entry == None:
                    entry = (None, None, None)
                if isinstance(res, dcp.parser.Ack):
                    entry = (t, res, entry[2])
                elif isinstance(res, dcp.parser.Reply):
                    entry = (t, entry[1], res)
                else:
                    entry = (t, entry[1], msg)
            else:
                if isinstance(res, dcp.parser.Command):
                    entry = (t, res)
                else:
                    entry = (t, msg)
            self._inQueue[key] = entry

    def _purgeInQueue(self):
        for k, v in self._inQueue.items():
            if v[0].hasExpired(self._purgeTimeout):
                del self._inQueue[k]

    def _nextCompletedMessage(self):
        for k, v in self._inQueue.items():
            isReply = k[0]
            if not isReply:
                del self._inQueue[k]
                return v[1]
            ack, reply = v[1], v[2]
            if reply != None:
                del self._inQueue[k]
                return reply
            elif ack != None and ack.error != 0:
                del self._inQueue[k]
                return ack
        return None

    def _messagesCompleted(self, keys):
        completed = []
        for k in keys:
            v = self._inQueue.get(k)
            if v != None:
                assert len(v) == 3
                ack, reply = v[1], v[2]
                if reply != None:
                    completed.append(True)
                    continue
                elif ack != None and ack.error != 0:
                    completed.append(True)
                    continue
            completed.append(False)
        return completed

    def wait(self, msgs=None, timeout=None):
        if timeout == None:
            timeout = self._timeout
        else:
            timeout = int(round(timeout * 1000.0))
        if msgs != None:
            if isinstance(msgs, collections.Iterable):
                keys = []
                for m in msgs:
                    keys.append((True, int(m.snr), bytes(m.destination)))
            else:
                keys = [(True, int(msgs.snr), bytes(msgs.destination))]

        timeLeft = 0
        t = QtCore.QElapsedTimer(); t.start()
        while (timeLeft >= 0):
            self._client.waitForReadyRead(timeLeft)
            self._parseAvailableMessages()

            if msgs == None:
                m = self._nextCompletedMessage()
                if m != None:
                    return m
            else:
                completed = self._messagesCompleted(keys)
                if all(completed):
                    res = []
                    for k in keys:
                        v = self._inQueue.pop(k)
                        assert len(v) == 3
                        ack, reply = v[1], v[2]
                        if reply != None:
                            res.append(reply)
                        elif ack != None:
                            res.append(ack)
                        else:
                            res.append(None)

                    if len(res) == 1 and (
                            not isinstance(msgs, collections.Iterable)):
                        return res[0]
                    else:
                        return res

            timeLeft = timeout - t.elapsed()
