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
#include <limits>

namespace Dcp {

/*! \class Client
    \brief A class that handles the communication with a DCP server.
 */


/*! \internal
    \brief Private data and implementation of Dcp::Client.
 */
class ClientPrivate
{
public:
    explicit ClientPrivate(Client *qq);
    virtual ~ClientPrivate();

    void readMessageFromSocket();
    void writeMessageToSocket(const Message &msg);
    void registerName(const QByteArray &deviceName);
    void incrementSnr() {
        snr = (snr < std::numeric_limits<quint32>::max()) ? snr+1 : 1;
    }

    static Client::State mapSocketState(QAbstractSocket::SocketState state);
    static Client::Error mapSocketError(QAbstractSocket::SocketError error);

    // private slots
    void _k_connected();
    void _k_socketStateChanged(QAbstractSocket::SocketState state);
    void _k_socketError(QAbstractSocket::SocketError error);
    void _k_readMessagesFromSocket();
    void _k_autoReconnectTimeout();

    // private data
    Client * const q;
    QTcpSocket *socket;
    QQueue<Message> inQueue;
    QString serverName;
    quint16 serverPort;
    QByteArray deviceName;
    QTimer *reconnectTimer;
    bool autoReconnect;
    bool connectionRequested;
    quint32 snr;
};

ClientPrivate::ClientPrivate(Client *qq)
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

ClientPrivate::~ClientPrivate()
{
    delete socket;
    delete reconnectTimer;
}

void ClientPrivate::readMessageFromSocket()
{
    if (socket->bytesAvailable() < FullHeaderSize)
        return;

    char pkgHeader[PacketHeaderSize];
    const quint32 *pMsgSize = reinterpret_cast<const quint32 *>(
                pkgHeader + PacketMsgSizePos);
    const quint32 *pOffset = reinterpret_cast<const quint32 *>(
                pkgHeader + PacketOffsetPos);

    socket->peek(pkgHeader, PacketHeaderSize);
    quint32 msgSize = qFromBigEndian(*pMsgSize);
    quint32 offset = qFromBigEndian(*pOffset);

    // not enough data (header + message)
    if (socket->bytesAvailable() < FullHeaderSize + msgSize)
        return;

    // remove packet header from the input buffer
    socket->read(pkgHeader, PacketHeaderSize);

    // read message data
    QByteArray rawMsg = socket->read(MessageHeaderSize + msgSize);

    // ignore multi-packet messages
    if (offset != 0 || rawMsg.size() != int(MessageHeaderSize + msgSize)) {
        qWarning("Dcp::Client: Ignoring incoming message. " \
                 "Multi-packet messages are currently not supported.");
    }
    else {
        inQueue.enqueue(Message::fromRawMsg(rawMsg));
        emit q->messageReceived();
    }
}

void ClientPrivate::writeMessageToSocket(const Message &msg)
{
    if (msg.isNull()) {
        qWarning("Dcp::Client::sendMessage: Ignoring invalid message.");
        return;
    }

    if (!msg.data().size() + FullHeaderSize > MaxPacketSize) {
        qWarning("Dcp::Client::sendMessage: Skipping large message. " \
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

    socket->write(pkgHeader, PacketHeaderSize);
    socket->write(msg.toRawMsg());
}

void ClientPrivate::registerName(const QByteArray &deviceName)
{
    Message msg(0, snr, deviceName, QByteArray(), "HELO");
    incrementSnr();
    writeMessageToSocket(msg);
    socket->flush();
}

Client::State ClientPrivate::mapSocketState(
    QAbstractSocket::SocketState state)
{
    switch (state)
    {
    case QAbstractSocket::UnconnectedState:
        return Client::UnconnectedState;
    case QAbstractSocket::HostLookupState:
        return Client::HostLookupState;
    case QAbstractSocket::ConnectingState:
        return Client::ConnectingState;
    case QAbstractSocket::ConnectedState:
        return Client::ConnectedState;
    case QAbstractSocket::ClosingState:
        return Client::ClosingState;
    case QAbstractSocket::BoundState:  // Servers sockets only
    case QAbstractSocket::ListeningState:  // Servers sockets only
    default:
        return Client::UnconnectedState;
    }
}

Client::Error ClientPrivate::mapSocketError(
    QAbstractSocket::SocketError error)
{
    switch (error)
    {
    case QAbstractSocket::ConnectionRefusedError:
        return Client::ConnectionRefusedError;
    case QAbstractSocket::RemoteHostClosedError:
        return Client::RemoteHostClosedError;
    case QAbstractSocket::HostNotFoundError:
        return Client::HostNotFoundError;
    case QAbstractSocket::SocketAccessError:
        return Client::SocketAccessError;
    case QAbstractSocket::SocketResourceError:
        return Client::SocketResourceError;
    case QAbstractSocket::SocketTimeoutError:
        return Client::SocketTimeoutError;
    case QAbstractSocket::NetworkError:
        return Client::NetworkError;
    case QAbstractSocket::UnsupportedSocketOperationError:
        return Client::UnsupportedSocketOperationError;
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
        return Client::UnknownSocketError;
    }
}

void ClientPrivate::_k_connected()
{
    // register device name and reemit signal
    registerName(deviceName);
    emit q->connected();
}

void ClientPrivate::_k_socketStateChanged(QAbstractSocket::SocketState state)
{
    if (autoReconnect && connectionRequested
                      && state == QAbstractSocket::UnconnectedState)
        reconnectTimer->start();
    else
        reconnectTimer->stop();

    emit q->stateChanged(mapSocketState(state));
}

void ClientPrivate::_k_socketError(QAbstractSocket::SocketError error)
{
    emit q->error(mapSocketError(error));
}

void ClientPrivate::_k_readMessagesFromSocket()
{
    // stop if not enough header data is available
    while (socket->bytesAvailable() >= FullHeaderSize)
        readMessageFromSocket();  // emits messageReceived()
}

void ClientPrivate::_k_autoReconnectTimeout()
{
    if (socket->state() == QAbstractSocket::UnconnectedState)
        socket->connectToHost(serverName, serverPort);
}

// ---------------------------------------------------------------------------

/*! \brief Constructor */
Client::Client(QObject *parent)
    : QObject(parent),
      d(new ClientPrivate(this))
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

/*! \brief Destructor */
Client::~Client()
{
    delete d;
}

/*!
    \brief Connect to DCP server.
 */
void Client::connectToServer(const QString &serverName, quint16 serverPort,
                             const QByteArray &deviceName)
{
    d->connectionRequested = true;
    d->serverName = serverName;
    d->serverPort = serverPort;
    d->deviceName = deviceName;
    d->socket->connectToHost(serverName, serverPort);
}

/*!
    \brief Disconnect from DCP server.
 */
void Client::disconnectFromServer()
{
    d->connectionRequested = false;
    d->socket->disconnectFromHost();
}

quint32 Client::nextSnr() const
{
    return d->snr;
}

void Client::setNextSnr(quint32 snr)
{
    d->snr = snr;
}

Message Client::sendMessage(const QByteArray &destination,
                            const QByteArray &data, quint16 flags)
{
    Message msg(flags, d->snr, d->deviceName, destination, data);
    d->incrementSnr();
    d->writeMessageToSocket(msg);
    return msg;
}

Message Client::sendMessage(const QByteArray &destination,
                            const QByteArray &data, quint8 userFlags,
                            quint8 dcpFlags)
{
    Message msg(0, d->snr, d->deviceName, destination, data);
    msg.setDcpFlags(dcpFlags);
    msg.setUserFlags(userFlags);
    d->incrementSnr();
    d->writeMessageToSocket(msg);
    return msg;
}

Message Client::sendMessage(quint32 snr, const QByteArray &destination,
                            const QByteArray &data, quint16 flags)
{
    Message msg(flags, snr, d->deviceName, destination, data);
    d->writeMessageToSocket(msg);
    return msg;
}

Message Client::sendMessage(quint32 snr, const QByteArray &destination,
                            const QByteArray &data, quint8 userFlags,
                            quint8 dcpFlags)
{
    Message msg(0, snr, d->deviceName, destination, data);
    msg.setDcpFlags(dcpFlags);
    msg.setUserFlags(userFlags);
    d->writeMessageToSocket(msg);
    return msg;
}


void Client::sendMessage(const Message &message)
{
    d->writeMessageToSocket(message);
}

int Client::messagesAvailable() const
{
    return d->inQueue.size();
}

Message Client::readMessage()
{
    return d->inQueue.isEmpty() ? Message() : d->inQueue.dequeue();
}

Client::State Client::state() const
{
    return ClientPrivate::mapSocketState(d->socket->state());
}

bool Client::isConnected() const
{
    return d->socket->state() == QAbstractSocket::ConnectedState;
}

bool Client::isUnconnected() const
{
    return d->socket->state() == QAbstractSocket::UnconnectedState;
}

Client::Error Client::error() const
{
    return ClientPrivate::mapSocketError(d->socket->error());
}

QString Client::errorString() const
{
    return d->socket->errorString();
}

QString Client::serverName() const
{
    return d->serverName;
}

quint16 Client::serverPort() const
{
    return d->serverPort;
}

QByteArray Client::deviceName() const
{
    return d->deviceName;
}

bool Client::autoReconnect() const
{
    return d->autoReconnect;
}

void Client::setAutoReconnect(bool enable)
{
    d->autoReconnect = enable;
    if (enable && d->connectionRequested
               && d->socket->state() == QAbstractSocket::UnconnectedState)
        d->reconnectTimer->start();
    else if (!enable)
        d->reconnectTimer->stop();
}

int Client::reconnectInterval() const
{
    return d->reconnectTimer->interval();
}

void Client::setReconnectInterval(int msecs)
{
    d->reconnectTimer->setInterval(msecs);
}

bool Client::waitForConnected(int msecs)
{
    // register device name if the connection was established
    bool ok = d->socket->waitForConnected(msecs);
    if (ok) d->registerName(d->deviceName);
    return ok;
}

bool Client::waitForDisconnected(int msecs)
{
    if (d->socket->state() != QAbstractSocket::UnconnectedState)
        return d->socket->waitForDisconnected(msecs);
    return true;
}

bool Client::waitForReadyRead(int msecs)
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

bool Client::waitForMessagesWritten(int msecs)
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
