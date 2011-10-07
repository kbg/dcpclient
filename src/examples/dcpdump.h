/*
 * Copyright (c) 2011 Kolja Glogowski
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef DCPDUMP_H
#define DCPDUMP_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QMap>
#include <QtNetwork/QAbstractSocket>

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
