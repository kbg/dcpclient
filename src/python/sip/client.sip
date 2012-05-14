namespace Dcp {

class Client : public QObject
{
%TypeHeaderCode
#include <dcpclient/client.h>
%End

public:
    enum State {
        UnconnectedState,
        HostLookupState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };

    enum Error {
        ConnectionRefusedError,
        RemoteHostClosedError,
        HostNotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,
        NetworkError,
        UnsupportedSocketOperationError,
        UnknownSocketError
    };

    explicit Client(QObject *parent /TransferThis/ = 0);
    virtual ~Client();

    void connectToServer(const QString &serverName, quint16 serverPort,
                         const QByteArray &deviceName) /ReleaseGIL/;
    void disconnectFromServer() /ReleaseGIL/;

    quint32 nextSnr() const;
    void setNextSnr(quint32 snr);

    Dcp::Message sendMessage(const QByteArray &destination,
        const QByteArray &data, quint16 flags = 0);
    Dcp::Message sendMessage(const QByteArray &destination,
        const QByteArray &data, quint8 dcpFlags /PyInt/,
        quint8 userFlags /PyInt/);
    Dcp::Message sendMessage(quint32 snr, const QByteArray &destination,
        const QByteArray &data, quint16 flags = 0);
    Dcp::Message sendMessage(quint32 snr, const QByteArray &destination,
        const QByteArray &data, quint8 dcpFlags /PyInt/,
        quint8 userFlags /PyInt/);
    void sendMessage(const Dcp::Message &message);

    int messagesAvailable() const;
    Dcp::Message readMessage();

    Dcp::Client::State state() const /ReleaseGIL/;
    bool isConnected() const /ReleaseGIL/;
    bool isUnconnected() const /ReleaseGIL/;

    Dcp::Client::Error error() const;
    QString errorString() const;

    QString serverName() const;
    QHostAddress serverAddress() const;
    quint16 serverPort() const;
    QByteArray deviceName() const;
    QHostAddress localAddress() const;
    quint16 localPort() const;

    bool autoReconnect() const;
    void setAutoReconnect(bool enable);
    int reconnectInterval() const;
    void setReconnectInterval(int msecs);

    bool waitForConnected(int msecs = 10000) /ReleaseGIL/;
    bool waitForDisconnected(int msecs = 10000) /ReleaseGIL/;
    bool waitForReadyRead(int msecs = 10000) /ReleaseGIL/;
    bool waitForMessagesWritten(int msecs = 10000) /ReleaseGIL/;

signals:
    void connected();
    void disconnected();
    void error(Dcp::Client::Error error);
    void stateChanged(Dcp::Client::State state);
    void messageReceived();

public:
    SIP_PYOBJECT __repr__() const /DocType="str"/;
%MethodCode
    QByteArray ba("Client('");
    ba += sipCpp->serverName() + "', ";
    ba += QByteArray::number(sipCpp->serverPort()) + ", '";
    ba += sipCpp->deviceName() + "')";

#if PY_MAJOR_VERSION >= 3
    sipRes = PyUnicode_FromString(ba.constData());
#else
    sipRes = PyString_FromString(ba.constData());
#endif
%End // __repr__()
};

}; // namespace Dcp
