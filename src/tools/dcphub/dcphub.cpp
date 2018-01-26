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

#include "dcphub.h"
#include "dcppacket.h"
#include <dcpclient/message.h>
#include <dcpclient/messageparser.h>
#include <dcpclient/version.h>
#include <QtCore/QtCore>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

QByteArray joined(const QList<QByteArray> &list, char sep = ' ')
{
    if (list.isEmpty())
        return QByteArray();
    QByteArray res = list.value(0);
    for (int i = 1; i < list.size(); ++i) {
        res += sep;
        res += list.value(i);
    }
    return res;
}

QByteArray deviceKey(const QByteArray &deviceName, bool percentDecode = false)
{
    int deviceNameLen = qMin(deviceName.size(), int(MessageDeviceNameSize));
    QByteArray key = percentDecode ?
            QByteArray::fromPercentEncoding(deviceName) : deviceName;
    key.resize(MessageDeviceNameSize);
    for (int i = deviceNameLen; i < MessageDeviceNameSize; ++i)
        key[i] = '\0';
    return key;
}

DcpHub::DcpHub(QObject *parent)
    : QObject(parent),
      cout(stdout, QIODevice::WriteOnly),
      cerr(stderr, QIODevice::WriteOnly),
      hexfmt(16, HexFormatter::ShowPosition | HexFormatter::ShowText, '.'),
      m_tcpServer(new QTcpServer(this)),
      m_serverDeviceName("dcphub"),
      m_printTimestamp(false),
      m_debugFlags(NoDebug)
{
    connect(m_tcpServer, SIGNAL(newConnection()), SLOT(newConnection()));
}

DcpHub::~DcpHub()
{
    close();
    delete m_tcpServer;
}

bool DcpHub::listen(const QHostAddress &address, quint16 port)
{
    if (!m_tcpServer->listen(address, port)) {
        cerr << ts() << "Error: Cannot listen to " << address.toString() << ":"
             << port << ". " << m_tcpServer->errorString() << "." << endl;
        return false;
    }

    cout << ts() << "Listening [" << m_tcpServer->serverAddress().toString()
         << ":" << m_tcpServer->serverPort() << "]." << endl;
    return true;
}

void DcpHub::close()
{
    QList<QTcpSocket *> socketList = m_socketMap.keys();
    foreach (QTcpSocket *socket, socketList)
        socket->disconnectFromHost();
    foreach (QTcpSocket *socket, socketList) {
        if (socket->state() != QAbstractSocket::UnconnectedState)
            socket->waitForDisconnected(3000);
    }
    m_tcpServer->close();
}

bool DcpHub::setDeviceName(const QByteArray &name)
{
    if (m_tcpServer->isListening() || name.isEmpty())
        return false;
    m_serverDeviceName = name;
    m_serverDeviceName.truncate(MessageDeviceNameSize);
    return true;
}

void DcpHub::newConnection()
{
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), SLOT(socketDisconnected()));
    connect(socket, SIGNAL(readyRead()), SLOT(socketReadyRead()));
    Q_ASSERT(!m_socketMap.contains(socket));

    ClientInfo clientInfo = {
        QByteArray(),
        socket->peerAddress(),
        socket->peerPort()
    };
    m_socketMap.insert(socket, clientInfo);

    cout << ts() << "New connection [" << clientInfo.address.toString()
         << ":" << clientInfo.port << "]." << endl;
}

void DcpHub::socketDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        qWarning("DcpHub::socketDisconnected(): Invalid sender.");
        return;
    }
    if (!m_socketMap.contains(socket)) {
        qWarning("DcpHub::socketDisconnected(): Unknown socket.");
        return;
    }

    const ClientInfo clientInfo = m_socketMap.value(socket);
    cout << "Disconnected device \"" << clientInfo.device << "\" ["
         << clientInfo.address.toString() << ":" << clientInfo.port << "]."
         << endl;

    if (!clientInfo.device.isEmpty()) {
        Q_ASSERT(m_deviceMap.value(clientInfo.device) == socket);
        m_deviceMap.remove(clientInfo.device);
    }
    m_socketMap.remove(socket);
    socket->deleteLater();
}

void DcpHub::socketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        qWarning("DcpHub::socketReadyRead(): Invalid sender.");
        return;
    }
    if (!m_socketMap.contains(socket)) {
        qWarning("DcpHub::socketReadyRead(): Unknown socket.");
        return;
    }

    DcpPacket packet;
    while (readNextPacket(socket, &packet))
    {
        if (m_debugFlags & MessageDebug)
            cout << packet.message() << endl;

        if (m_debugFlags & PacketDebug)
            cout << hexfmt(packet.data()) << endl;

        // register device if neccessary, disconnect on error
        if (m_socketMap.value(socket).device.isEmpty()) {
            if (!registerDeviceName(socket, packet.source())) {
                socket->disconnectFromHost();
                return;
            }
        }

        processPacket(packet);
    }
}

bool DcpHub::readNextPacket(QTcpSocket *socket, DcpPacket *packet)
{
    Q_ASSERT(socket);
    Q_ASSERT(packet);
    packet->clear();

    if (socket->bytesAvailable() < FullHeaderSize)
        return false;

    char header[FullHeaderSize];
    socket->peek(header, FullHeaderSize);
    quint32 msgDataSize = qFromBigEndian(*reinterpret_cast<const quint32 *>(
        header + PacketHeaderSize + MessageDataLenPos));
    quint32 pkgSize = FullHeaderSize + msgDataSize;

    // invalid packet size -> disconnect
    if (pkgSize > MaxPacketSize) {
        socket->disconnectFromHost();
        return false;
    }

    // check if the full packet is available
    if (socket->bytesAvailable() < pkgSize)
        return false;

    packet->setData(socket->read(pkgSize));
    return true;
}

bool DcpHub::registerDeviceName(QTcpSocket *socket, const QByteArray &name)
{
    Q_ASSERT(socket);

    SocketMap::iterator iter = m_socketMap.find(socket);
    if (iter == m_socketMap.end())
        return false;

    ClientInfo &ci = iter.value();
    Q_ASSERT(ci.device.isEmpty());

    if (isNullDeviceName(name) || isServerDeviceName(name)) {
        cerr << "Device trying to register invalid name ["
             << ci.address.toString() << ":" << ci.port << "]." << endl;
        return false;
    }

    if (m_deviceMap.contains(name)) {
        cerr << "Device name \"" << name << "\" already exists ["
             << ci.address.toString() << ":" << ci.port << "]." << endl;
        return false;
    }

    ci.device = name;
    m_deviceMap[name] = socket;

    cout << "Registered device \"" << name << "\" ["
         << ci.address.toString() << ":" << ci.port << "]." << endl;
    return true;
}

void DcpHub::processPacket(const DcpPacket &packet)
{
    QByteArray device = packet.destination();

    // send packet to its destination device
    QTcpSocket *socket = m_deviceMap.value(device, 0);
    if (socket) {
        socket->write(packet.data());
        return;
    }

    // ignore packets with unknown device names
    if (!isServerDeviceName(device))
        return;

    // handle packets addressed to the server itself
    Dcp::Message msg = packet.message();
    if (!msg.isNull() && !msg.isReply())
        handleCommand(msg);
}

void DcpHub::sendMessage(QTcpSocket *socket, const Dcp::Message &msg)
{
    Q_ASSERT(socket);
    if (msg.isNull()) {
        qWarning("DcpHub::sendMessage(): Ignoring invalid message.");
        return;
    }
    if (!msg.data().size() + FullHeaderSize > MaxPacketSize) {
        qWarning("DcpHub::sendMessage(): Skipping large message. " \
                 "Multi-packet messages are currently not supported.");
        return;
    }

    char pkgHeader[PacketHeaderSize];
    quint32 *pMsgSize = reinterpret_cast<quint32 *>(
                pkgHeader + PacketMsgSizePos);
    quint32 *pOffset = reinterpret_cast<quint32 *>(
                pkgHeader + PacketOffsetPos);

    *pMsgSize = qToBigEndian(static_cast<quint32>(msg.data().size()));
    *pOffset = 0;

    if (m_debugFlags & MessageDebug)
        cout << msg << endl;

    if (m_debugFlags & PacketDebug) {
        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        buf.write(pkgHeader, PacketHeaderSize);
        buf.write(msg.toByteArray());
        cout << hexfmt(buf.data()) << endl;
        socket->write(buf.data());
    } else {
        socket->write(pkgHeader, PacketHeaderSize);
        socket->write(msg.toByteArray());
    }
}

void DcpHub::handleCommand(const Dcp::Message &msg)
{
    Q_ASSERT(!msg.isNull() && !msg.isReply());
    Q_ASSERT(isServerDeviceName(msg.destination()));
    QTcpSocket *socket = m_deviceMap.value(deviceKey(msg.source()), 0);
    if (!socket) {
        qWarning("DcpHub::handleCommand(): Unknown socket.");
        return;
    }

    Dcp::CommandParser cmd;
    if (!cmd.parse(msg))
        return;

    QByteArray identifier = cmd.identifier();
    QList<QByteArray> args = cmd.arguments();
    Dcp::CommandParser::CmdType cmdType = cmd.cmdType();
    if (cmdType == Dcp::CommandParser::GetCmd)
    {
        // get devlist
        //     returns: dev1 dev2 ...
        if (identifier == "devlist")
        {
            if (cmd.hasArguments()) {
                sendMessage(socket, msg.ackMessage(Dcp::AckParameterError));
                return;
            }
            sendMessage(socket, msg.ackMessage());
            sendMessage(socket, msg.replyMessage(joined(deviceList(true))));
            return;
        }

        // get devinfo [dev1 [dev2 [...]]]
        //     returns: [dev1 addr1 port1 [dev2 addr2 port2 [...]]] | FIN
        //     errorcodes: -1 -> at least one device is unknown
        //     notes: if no device is specified all devices are returned
        if (identifier == "devinfo")
        {
            sendMessage(socket, msg.ackMessage());
            if (args.isEmpty())
                args = deviceList(true);
            int errorCode = 0;
            QList<QByteArray> result;
            foreach (QByteArray device, args)
            {
                QByteArray key = deviceKey(device, true);
                QTcpSocket *devSocket = m_deviceMap.value(key, 0);
                if (devSocket) {
                    Q_ASSERT(m_socketMap.contains(devSocket));
                    ClientInfo info = m_socketMap.value(devSocket);
                    result.append(device);
                    result.append(info.address.toString().toAscii());
                    result.append(QByteArray::number(info.port));
                }
                else
                    errorCode = -1;
            }
            sendMessage(socket, msg.replyMessage(joined(result), errorCode));
            return;
        }

        // get debug
        //     returns: ( none | msg | pkg | full )
        if (identifier == "debug")
        {
            if (cmd.hasArguments()) {
                sendMessage(socket, msg.ackMessage(Dcp::AckParameterError));
                return;
            }
            sendMessage(socket, msg.ackMessage());

            QByteArray mode;
            switch (m_debugFlags) {
            case NoDebug:
                mode = "none";
                break;
            case MessageDebug:
                mode = "msg";
                break;
            case PacketDebug:
                mode = "pkg";
                break;
            case FullDebug:
                mode = "full";
                break;
            }
            sendMessage(socket, msg.replyMessage(mode));
            return;
        }

        // get version
        //     returns: <version>
        if (identifier == "version")
        {
            if (cmd.hasArguments()) {
                sendMessage(socket, msg.ackMessage(Dcp::AckParameterError));
                return;
            }
            sendMessage(socket, msg.ackMessage());
            sendMessage(socket, msg.replyMessage(DCPCLIENT_VERSION_STRING));
            return;
        }
    }
    else if (cmdType == Dcp::CommandParser::SetCmd)
    {
        // set nop
        //     returns: FIN
        if (identifier == "nop")
        {
            if (cmd.hasArguments()) {
                sendMessage(socket, msg.ackMessage(Dcp::AckParameterError));
                return;
            }
            sendMessage(socket, msg.ackMessage());
            sendMessage(socket, msg.replyMessage());
            return;
        }

        // set debug ( none | msg | pkg | full )
        //     returns: FIN
        if (identifier == "debug")
        {
            if (args.size() != 1) {
                sendMessage(socket, msg.ackMessage(Dcp::AckParameterError));
                return;
            }
            QByteArray mode = args[0];
            DebugFlags flags;
            if (mode == "none" || mode == "off" || mode == "0")
                flags = NoDebug;
            else if (mode == "msg" || mode == "on" || mode == "1")
                flags = MessageDebug;
            else if (mode == "pkg")
                flags = PacketDebug;
            else if (mode == "full")
                flags = FullDebug;
            else {
                sendMessage(socket, msg.ackMessage(Dcp::AckParameterError));
                return;
            }
            sendMessage(socket, msg.ackMessage());
            m_debugFlags = flags;
            sendMessage(socket, msg.replyMessage());
            return;
        }
    }

    sendMessage(socket, msg.ackMessage(Dcp::AckUnknownCommandError));
}

QString DcpHub::ts() const
{
    if (m_printTimestamp)
        return QDateTime::currentDateTimeUtc().toString("yyyyMMdd hh:mm:ss  ");
    else
        return QString();
}

bool DcpHub::isNullDeviceName(const QByteArray &name) const
{
    //Q_ASSERT(name.size() == MessageDeviceNameSize);
    const int count = name.size();
    for (int i = 0; i < count; ++i)
        if (name[i] != '\0')
            return false;
    return true;
}

bool DcpHub::isServerDeviceName(const QByteArray &name) const
{
    //Q_ASSERT(name.size() == MessageDeviceNameSize);
    int i, count = qMin(name.size(), m_serverDeviceName.size());
    for (i = 0; i < count; ++i)
        if (name[i] != m_serverDeviceName[i])
            return false;
    for (count = name.size(); i < count; ++i)
        if (name[i] != '\0')
            return false;
    return true;
}

QList<QByteArray> DcpHub::deviceList(bool percentEncoded)
{
    QList<QByteArray> devList = m_deviceMap.keys();
    QMutableListIterator<QByteArray> it(devList);
    while (it.hasNext()) {
        QByteArray &dev = it.next();
        int n = dev.size();
        while (n > 0 && (dev[n-1] == '\0'))
            n--;
        dev.truncate(n);
        if (percentEncoded)
            dev = Dcp::percentEncodeSpaces(dev);
    }
    return devList;
}
