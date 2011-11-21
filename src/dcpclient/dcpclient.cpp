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

#include "dcpclient.h"
#include "dcpclient_p.h"
#include "dcpmessage.h"
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QQueue>
#include <QtCore/QtEndian>
#include <QtCore/QElapsedTimer>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>

namespace Dcp {

class DcpClientPrivate
{
public:
    explicit DcpClientPrivate(DcpClient *qq);
    virtual ~DcpClientPrivate();

    void readMessageFromSocket();
    void writeMessageToSocket(const DcpMessage &msg);
    void registerName(const QByteArray &deviceName);

    static DcpClient::State mapSocketState(QAbstractSocket::SocketState state);
    static DcpClient::Error mapSocketError(QAbstractSocket::SocketError error);

    // private slots
    void _k_connected();
    void _k_socketStateChanged(QAbstractSocket::SocketState state);
    void _k_socketError(QAbstractSocket::SocketError error);
    void _k_readMessagesFromSocket();
    void _k_autoReconnectTimeout();

    // private data
    DcpClient * const q;
    QTcpSocket *socket;
    QQueue<DcpMessage> inQueue;
    QString serverName;
    quint16 serverPort;
    QByteArray deviceName;
    QTimer *reconnectTimer;
    bool autoReconnect;
    bool connectionRequested;
    quint32 snr;
};

DcpClientPrivate::DcpClientPrivate(DcpClient *qq)
    : q(qq),
      socket(new QTcpSocket),
      serverPort(0),
      reconnectTimer(new QTimer),
      autoReconnect(false),
      connectionRequested(false),
      snr(0)
{
    reconnectTimer->setInterval(30000);
}

DcpClientPrivate::~DcpClientPrivate()
{
    delete socket;
    delete reconnectTimer;
}

void DcpClientPrivate::readMessageFromSocket()
{
    if (socket->bytesAvailable() < DcpFullHeaderSize)
        return;

    char pkgHeader[DcpPacketHeaderSize];
    const quint32 *pMsgSize = reinterpret_cast<const quint32 *>(
                pkgHeader + DcpPacketMsgSizePos);
    const quint32 *pOffset = reinterpret_cast<const quint32 *>(
                pkgHeader + DcpPacketOffsetPos);

    socket->peek(pkgHeader, DcpPacketHeaderSize);
    quint32 msgSize = qFromBigEndian(*pMsgSize);
    quint32 offset = qFromBigEndian(*pOffset);

    // not enough data (header + message)
    if (socket->bytesAvailable() < DcpFullHeaderSize + msgSize)
        return;

    // remove packet header from the input buffer
    socket->read(pkgHeader, DcpPacketHeaderSize);

    // read message data
    QByteArray rawMsg = socket->read(DcpMessageHeaderSize + msgSize);

    // ignore multi-packet messages
    if (offset != 0 || rawMsg.size() != int(DcpMessageHeaderSize + msgSize)) {
        qWarning("DcpConnection: Ignoring incoming message. " \
                 "Multi-packet messages are currently not supported.");
    }
    else {
        inQueue.enqueue(DcpMessage::fromRawMsg(rawMsg));
        emit q->messageReceived();
    }
}

void DcpClientPrivate::writeMessageToSocket(const DcpMessage &msg)
{
    if (msg.isNull()) {
        qWarning("DcpConnection::sendMessage: Ignoring invalid message.");
        return;
    }

    if (!msg.data().size() + DcpFullHeaderSize > DcpMaxPacketSize) {
        qWarning("DcpConnection::sendMessage: Skipping large message. " \
                 "Multi-packet messages are currently not supported.");
        return;
    }

    char pkgHeader[DcpPacketHeaderSize];
    quint32 *pMsgSize = reinterpret_cast<quint32 *>(
                pkgHeader + DcpPacketMsgSizePos);
    quint32 *pOffset = reinterpret_cast<quint32 *>(
                pkgHeader + DcpPacketOffsetPos);

    *pMsgSize = qToBigEndian(static_cast<quint32>(msg.data().size()));
    *pOffset = 0;

    socket->write(pkgHeader, DcpPacketHeaderSize);
    socket->write(msg.toRawMsg());
}

void DcpClientPrivate::registerName(const QByteArray &deviceName)
{
    DcpMessage msg(0, 0, deviceName, QByteArray(), "HELO");
    writeMessageToSocket(msg);
    socket->flush();
}

DcpClient::State DcpClientPrivate::mapSocketState(
    QAbstractSocket::SocketState state)
{
    switch (state)
    {
    case QAbstractSocket::UnconnectedState:
        return DcpClient::UnconnectedState;
    case QAbstractSocket::HostLookupState:
        return DcpClient::HostLookupState;
    case QAbstractSocket::ConnectingState:
        return DcpClient::ConnectingState;
    case QAbstractSocket::ConnectedState:
        return DcpClient::ConnectedState;
    case QAbstractSocket::ClosingState:
        return DcpClient::ClosingState;
    case QAbstractSocket::BoundState:  // Servers sockets only
    case QAbstractSocket::ListeningState:  // Servers sockets only
    default:
        return DcpClient::UnconnectedState;
    }
}

DcpClient::Error DcpClientPrivate::mapSocketError(
    QAbstractSocket::SocketError error)
{
    switch (error)
    {
    case QAbstractSocket::ConnectionRefusedError:
        return DcpClient::ConnectionRefusedError;
    case QAbstractSocket::RemoteHostClosedError:
        return DcpClient::RemoteHostClosedError;
    case QAbstractSocket::HostNotFoundError:
        return DcpClient::HostNotFoundError;
    case QAbstractSocket::SocketAccessError:
        return DcpClient::SocketAccessError;
    case QAbstractSocket::SocketResourceError:
        return DcpClient::SocketResourceError;
    case QAbstractSocket::SocketTimeoutError:
        return DcpClient::SocketTimeoutError;
    case QAbstractSocket::NetworkError:
        return DcpClient::NetworkError;
    case QAbstractSocket::UnsupportedSocketOperationError:
        return DcpClient::UnsupportedSocketOperationError;
    case QAbstractSocket::UnknownSocketError:
    case QAbstractSocket::DatagramTooLargeError:  // QUdpSocket only
    case QAbstractSocket::AddressInUseError:  // QUdpSocket only
    case QAbstractSocket::SocketAddressNotAvailableError:  // QUdpSocket only
    case QAbstractSocket::UnfinishedSocketOperationError:  // QAbstractSocketEngine only
    case QAbstractSocket::ProxyAuthenticationRequiredError:  // No proxy support
    case QAbstractSocket::SslHandshakeFailedError:  // QSslSocket only
    case QAbstractSocket::ProxyConnectionRefusedError:  // No proxy support
    case QAbstractSocket::ProxyConnectionClosedError:  // No proxy support
    case QAbstractSocket::ProxyConnectionTimeoutError:  // No proxy support
    case QAbstractSocket::ProxyNotFoundError:  // No proxy support
    case QAbstractSocket::ProxyProtocolError:  // No proxy support
    default:
        return DcpClient::UnknownSocketError;
    }
}

void DcpClientPrivate::_k_connected()
{
    // register device name and reemit signal
    registerName(deviceName);
    emit q->connected();
}

void DcpClientPrivate::_k_socketStateChanged(QAbstractSocket::SocketState state)
{
    if (autoReconnect && connectionRequested
                      && state == QAbstractSocket::UnconnectedState)
        reconnectTimer->start();
    else
        reconnectTimer->stop();

    emit q->stateChanged(mapSocketState(state));
}

void DcpClientPrivate::_k_socketError(QAbstractSocket::SocketError error)
{
    emit q->error(mapSocketError(error));
}

void DcpClientPrivate::_k_readMessagesFromSocket()
{
    // stop if not enough header data is available
    while (socket->bytesAvailable() >= DcpFullHeaderSize)
        readMessageFromSocket();  // emits messageReceived()
}

void DcpClientPrivate::_k_autoReconnectTimeout()
{
    if (socket->state() == QAbstractSocket::UnconnectedState)
        socket->connectToHost(serverName, serverPort);
}

// ---------------------------------------------------------------------------

DcpClient::DcpClient(QObject *parent)
    : QObject(parent),
      d(new DcpClientPrivate(this))
{
    connect(d->socket, SIGNAL(connected()), SLOT(_k_connected()));
    connect(d->socket, SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(_k_socketError(QAbstractSocket::SocketError)));
    connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            SLOT(_k_socketStateChanged(QAbstractSocket::SocketState)));
    connect(d->socket, SIGNAL(readyRead()), SLOT(_k_readMessagesFromSocket()));
    connect(d->reconnectTimer, SIGNAL(timeout()), SLOT(_k_autoReconnectTimeout()));
}

DcpClient::~DcpClient()
{
    delete d;
}

void DcpClient::connectToServer(const QString &serverName, quint16 serverPort,
                                const QByteArray &deviceName)
{
    d->connectionRequested = true;
    d->serverName = serverName;
    d->serverPort = serverPort;
    d->deviceName = deviceName;
    d->socket->connectToHost(serverName, serverPort);
}

void DcpClient::disconnectFromServer()
{
    d->connectionRequested = false;
    d->socket->disconnectFromHost();
}

quint32 DcpClient::nextSnr() const
{
    return d->snr;
}

void DcpClient::setNextSnr(quint32 snr)
{
    d->snr = snr;
}

quint32 DcpClient::incrementSnr()
{
    return d->snr++;
}

quint32 DcpClient::sendMessage(const QByteArray &destination,
                               const QByteArray &data, quint16 flags)
{
    DcpMessage msg(flags, d->snr++, d->deviceName, destination, data);
    d->writeMessageToSocket(msg);
    return msg.snr();
}

quint32 DcpClient::sendMessage(const QByteArray &destination,
                               const QByteArray &data, quint8 userFlags,
                               quint8 dcpFlags)
{
    DcpMessage msg(0, d->snr++, d->deviceName, destination, data);
    msg.setDcpFlags(dcpFlags);
    msg.setUserFlags(userFlags);
    d->writeMessageToSocket(msg);
    return msg.snr();
}

void DcpClient::sendMessage(quint32 snr, const QByteArray &destination,
                            const QByteArray &data, quint16 flags)
{
    DcpMessage msg(flags, snr, d->deviceName, destination, data);
    d->writeMessageToSocket(msg);
}

void DcpClient::sendMessage(quint32 snr, const QByteArray &destination,
                            const QByteArray &data, quint8 userFlags,
                            quint8 dcpFlags)
{
    DcpMessage msg(0, snr, d->deviceName, destination, data);
    msg.setDcpFlags(dcpFlags);
    msg.setUserFlags(userFlags);
    d->writeMessageToSocket(msg);
}


void DcpClient::sendMessage(const DcpMessage &message)
{
    d->writeMessageToSocket(message);
}

int DcpClient::messagesAvailable() const
{
    return d->inQueue.size();
}

DcpMessage DcpClient::readMessage()
{
    return d->inQueue.isEmpty() ? DcpMessage() : d->inQueue.dequeue();
}

DcpClient::State DcpClient::state() const
{
    return DcpClientPrivate::mapSocketState(d->socket->state());
}

bool DcpClient::isConnected() const
{
    return d->socket->state() == QAbstractSocket::ConnectedState;
}

bool DcpClient::isUnconnected() const
{
    return d->socket->state() == QAbstractSocket::UnconnectedState;
}

DcpClient::Error DcpClient::error() const
{
    return DcpClientPrivate::mapSocketError(d->socket->error());
}

QString DcpClient::errorString() const
{
    return d->socket->errorString();
}

QString DcpClient::serverName() const
{
    return d->serverName;
}

quint16 DcpClient::serverPort() const
{
    return d->serverPort;
}

QByteArray DcpClient::deviceName() const
{
    return d->deviceName;
}

bool DcpClient::autoReconnect() const
{
    return d->autoReconnect;
}

void DcpClient::setAutoReconnect(bool enable)
{
    d->autoReconnect = enable;
    if (enable && d->connectionRequested
               && d->socket->state() == QAbstractSocket::UnconnectedState)
        d->reconnectTimer->start();
    else if (!enable)
        d->reconnectTimer->stop();
}

int DcpClient::reconnectInterval() const
{
    return d->reconnectTimer->interval();
}

void DcpClient::setReconnectInterval(int msecs)
{
    d->reconnectTimer->setInterval(msecs);
}

bool DcpClient::waitForConnected(int msecs)
{
    // register device name if the connection was established
    bool ok = d->socket->waitForConnected(msecs);
    if (ok) d->registerName(d->deviceName);
    return ok;
}

bool DcpClient::waitForDisconnected(int msecs)
{
    if (d->socket->state() != QAbstractSocket::UnconnectedState)
        return d->socket->waitForDisconnected(msecs);
    return true;
}

bool DcpClient::waitForReadyRead(int msecs)
{
    QElapsedTimer stopWatch;
    stopWatch.start();

    while (messagesAvailable() == 0)
    {
        int msecsLeft = timeoutValue(msecs, stopWatch.elapsed());
        if (!d->socket->waitForReadyRead(msecsLeft))
            return false;

        // try to read messages
        d->_k_readMessagesFromSocket();

        if (msecsLeft == 0)
            break;
    }

    return messagesAvailable() != 0;
}

bool DcpClient::waitForMessagesWritten(int msecs)
{
    QElapsedTimer stopWatch;
    stopWatch.start();

    while(d->socket->bytesToWrite() != 0)
    {
        int msecsLeft = timeoutValue(msecs, stopWatch.elapsed());
        if (msecsLeft == 0)
            break;

        if (!d->socket->waitForBytesWritten(msecsLeft))
            return false;
    }

    return d->socket->bytesToWrite() == 0;
}

} // namespace Dcp

#include "dcpclient.moc"
