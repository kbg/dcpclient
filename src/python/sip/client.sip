class Client : public QObject
{
%TypeHeaderCode
#include <dcpclient/client.h>
using namespace Dcp;
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

    explicit Client(QObject *parent = 0);
    virtual ~Client();

    void connectToServer(const QString &serverName, quint16 serverPort,
                         const QByteArray &deviceName);
    void disconnectFromServer();

    quint32 nextSnr() const;
    void setNextSnr(quint32 snr);

    Message sendMessage(const QByteArray &destination, const QByteArray &data,
                        quint16 flags = 0);
    Message sendMessage(const QByteArray &destination, const QByteArray &data,
                        quint8 dcpFlags /PyInt/, quint8 userFlags /PyInt/);
    Message sendMessage(quint32 snr, const QByteArray &destination,
                        const QByteArray &data, quint16 flags = 0);
    Message sendMessage(quint32 snr, const QByteArray &destination,
                        const QByteArray &data, quint8 dcpFlags /PyInt/,
                        quint8 userFlags /PyInt/);
    void sendMessage(const Message &message);

    int messagesAvailable() const;
    Message readMessage();

    Client::State state() const;
    bool isConnected() const;
    bool isUnconnected() const;

    Client::Error error() const;
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

    bool waitForConnected(int msecs = 10000);
    bool waitForDisconnected(int msecs = 10000);
    bool waitForReadyRead(int msecs = 10000);
    bool waitForMessagesWritten(int msecs = 10000);

signals:
    void connected();
    void disconnected();
    void error(/*Dcp::*/Client::Error error);
    void stateChanged(/*Dcp::*/Client::State state);
    void messageReceived();
};
