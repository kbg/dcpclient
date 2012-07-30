/*
 * Copyright (c) 2012 Kolja Glogowski
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

#ifndef DCPHUB_H
#define DCPHUB_H

#include "hexformatter.h"
#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QMap>
#include <QtCore/QTextStream>
#include <QtNetwork/QHostAddress>

class QTcpServer;
class QTcpSocket;
class DcpPacket;

namespace Dcp {
    class Message;
}

class DcpHub : public QObject
{
    Q_OBJECT

public:
    explicit DcpHub(QObject *parent = 0);
    ~DcpHub();

    bool listen(const QHostAddress &address = QHostAddress::Any,
                quint16 port = 2001);
    void close();

protected slots:
    void newConnection();
    void socketDisconnected();
    void socketReadyRead();

protected:
    bool readNextPacket(QTcpSocket *socket, DcpPacket *packet);
    bool registerDeviceName(QTcpSocket *socket, const QByteArray &name);
    void processPacket(const DcpPacket &packet);
    void sendMessage(QTcpSocket *socket, const Dcp::Message &msg);
    void handleCommand(const Dcp::Message &msg);

    QString ts() const;
    bool isNullDeviceName(const QByteArray &name) const;
    bool isServerDeviceName(const QByteArray &name) const;
    QList<QByteArray> deviceList(bool percentEncoded);

    struct ClientInfo {
        QByteArray device;
        QHostAddress address;
        quint16 port;
    };

    typedef QMap<QTcpSocket *, ClientInfo> SocketMap;
    typedef QMap<QByteArray, QTcpSocket *> DeviceMap;

    enum DebugFlags {
        NoDebug = 0x00,
        MessageDebug = 0x01,
        PacketDebug = 0x02,
        FullDebug = MessageDebug | PacketDebug
    };

private:
    Q_DISABLE_COPY(DcpHub)
    QTextStream cout, cerr;
    HexFormatter hexfmt;
    QTcpServer * const m_tcpServer;
    SocketMap m_socketMap;
    DeviceMap m_deviceMap;
    QByteArray m_serverDeviceName;
    bool m_printTimestamp;
    DebugFlags m_debugFlags;
};

#endif // DCPHUB_H
