#ifndef ECHOSERVER_H
#define ECHOSERVER_H

#include "dcp.h"
#include <QObject>

class QString;
class QTimer;


class EchoServer : public QObject
{
    Q_OBJECT

public:
    explicit EchoServer(QObject *parent = 0);
    virtual ~EchoServer();

public slots:
    void connectToServer(const QString &hostName, quint16 port = 2001);
    void disconnectFromServer();

protected slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);
    void messageReady();
    void reconnectTimer_timeout();

private:
    Q_DISABLE_COPY(EchoServer)
    DcpConnection *m_dcp;
    QTimer *m_reconnectTimer;
    QString m_serverName;
    quint16 m_serverPort;
};

#endif // ECHOSERVER_H
