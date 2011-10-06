#ifndef DCPCONNECTION_H
#define DCPCONNECTION_H

#include "dcpmessage.h"
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtNetwork/QAbstractSocket>

class QByteArray;
class QTcpSocket;

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

    bool waitForConnected(int msecs = 10000);
    bool waitForDisconnected(int msecs = 10000);
    bool waitForReadyRead(int msecs = 10000);
    bool waitForMessagesWritten(int msecs = 10000);

signals:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);
    void readyRead();

private slots:
    //! \todo Use Q_PRIVATE_SLOT().
    void readMessagesFromSocket();

private:
    Q_DISABLE_COPY(DcpConnection)

    //! \todo Use Q_DECLARE_PRIVATE().
    QTcpSocket *m_socket;
    QQueue<DcpMessage> m_inQueue;
};

#endif // DCPCONNECTION_H
