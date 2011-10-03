#ifndef DCPDUMP_H
#define DCPDUMP_H

#include "dcp.h"
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QMap>

class QTimer;
class DcpConnection;

class DcpDump : public QObject
{
    Q_OBJECT

public:
    explicit DcpDump(QObject *parent = 0);
    virtual ~DcpDump();

    void connectToServer(const QByteArray &deviceName,
                         const QString &serverName, quint16 serverPort);
    void disconnectFromServer();

    void setReconnectInterval(int msec);
    void setDeviceMap(const QMap<QByteArray, QByteArray> &deviceMap);

protected slots:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);
    void messageReady();
    void reconnectTimer_timeout();

private:
    Q_DISABLE_COPY(DcpDump)
    DcpConnection *m_dcp;
    QString m_serverName;
    quint16 m_serverPort;
    QByteArray m_deviceName;
    QMap<QByteArray, QByteArray> m_deviceMap;
    QTimer *m_reconnectTimer;
    bool m_reconnect;
};

#endif // DCPDUMP_H
