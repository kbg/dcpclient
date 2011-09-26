#ifndef DCP_H
#define DCP_H

#include <QObject>
#include <QTcpSocket>

class QByteArray;
class QTcpSocket;

class DcpMessage
{
public:
    DcpMessage();
//    DcpMessage(const DcpMessage &other);
//    DcpMessage(quint16 flags, quint32 snr, const QByteArray &source,
//               const QByteArray &destination, const QByteArray &data);

//    quint16 flags() const;
//    void setFlags(quint16 flags);

//    quint32 snr() const;
//    void setSnr(quint32 snr);

//    QByteArray source() const;
//    void setSource(const QByteArray &source);

//    QByteArray destination() const;
//    void setDestination(const QByteArray &destination);

//    QByteArray data() const;
//    void setData(const QByteArray &data);

private:
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

    QAbstractSocket::SocketError error() const;
    QString errorString() const;
    QAbstractSocket::SocketState state() const;

    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);

    void sendMessage(const QByteArray &rawData);

signals:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);

private:
    Q_DISABLE_COPY(DcpConnection)
    QTcpSocket *m_socket;
};


#endif // DCP_H
