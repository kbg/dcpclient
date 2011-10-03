#ifndef DCP_H
#define DCP_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>

class QByteArray;
class QTcpSocket;

class DcpMessage
{
public:
    DcpMessage();
    DcpMessage(const DcpMessage &other);
    explicit DcpMessage(const QByteArray &rawMsg);
    DcpMessage(quint16 flags, quint32 snr, const QByteArray &source,
               const QByteArray &destination, const QByteArray &data);

    void clear();
    bool isNull() const;

    quint16 flags() const;
    void setFlags(quint16 flags);

    quint32 snr() const;
    void setSnr(quint32 snr);

    QByteArray source() const;
    void setSource(const QByteArray &source);

    QByteArray destination() const;
    void setDestination(const QByteArray &destination);

    QByteArray data() const;
    void setData(const QByteArray &data);

    QByteArray toRawMsg() const;
    static DcpMessage fromRawMsg(const QByteArray &rawMsg);

private:
    void init(const QByteArray &rawMsg);
    void init(quint16 flags, quint32 snr, const QByteArray &source,
              const QByteArray &destination, const QByteArray &data);

private:
    bool m_null;
    quint16 m_flags;
    quint32 m_snr;
    QByteArray m_source;
    QByteArray m_destination;
    QByteArray m_data;
};


class DcpConnection : public QObject
{
    Q_OBJECT

public:
    explicit DcpConnection(QObject *parent = 0);

    void connectToServer(const QString &hostName, quint16 port = 2001);
    void disconnectFromServer();
    void registerName(const QByteArray &deviceName);

    void writeMessage(const DcpMessage &msg);
    bool flush();

    int messagesAvailable() const;
    DcpMessage readMessage();

    QAbstractSocket::SocketError error() const;
    QString errorString() const;
    QAbstractSocket::SocketState state() const;

    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

signals:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);
    void readyRead();

private slots:
    void onSocketReadyRead();

private:
    Q_DISABLE_COPY(DcpConnection)
    QTcpSocket *m_socket;
    QQueue<DcpMessage> m_inQueue;
};


#endif // DCP_H
