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

#include "client.h"
#include "message.h"
#include "dcpclient_p.h"
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QQueue>
#include <QtCore/QtEndian>
#include <QtCore/QElapsedTimer>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>
#include <limits>

namespace Dcp {

/*! \class Client
    \brief Client class for communicating with a DCP server.

    \fn Client::connected()
    \brief This signal is emitted after connectToServer() has been called
           and a connection has been successfully established.
    \sa connectToServer(), disconnected()

    \fn Client::disconnected()
    \brief This signal is emitted when client has been disconnected from
           the server.
    \sa connectToServer(), disconnectFromServer(), connected()

    \fn Client::error(Client::Error error)
    \brief This signal is emitted after an error occurred. The \a error
           parameter describes the type of error that occurred.
    \sa error(), errorString()

    \fn Client::stateChanged(Client::State state)
    \brief This signal is emitted whenever the Client's state changes.
           The \a state parameter is the new state.
    \sa state()

    \fn void Client::messageReceived()
    \brief This signal is emitted once every time a new message is available
           for reading.
    \sa messagesAvailable(), readMessage()
 */


/*! \internal
    \brief Private data and implementation of Dcp::Client.
 */
class ClientPrivate
{
public:
    explicit ClientPrivate(Client *qq);
    virtual ~ClientPrivate();

    bool readNextMessageFromSocket();
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

/*! \todo Check for (FullHeaderSize + MessageDataLen) instead of
 *      (FullHeaderSize + PacketMsgSize), i.e. don't use msgSize but
 *      something like pkgSize.
 */
bool ClientPrivate::readNextMessageFromSocket()
{
    if (socket->bytesAvailable() < FullHeaderSize)
        return false;

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
        return false;

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
        inQueue.enqueue(Message::fromByteArray(rawMsg));
        emit q->messageReceived();
    }

    return true;
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
    socket->write(msg.toByteArray());
}

void ClientPrivate::registerName(const QByteArray &deviceName)
{
    Message msg(snr, deviceName, QByteArray(), "HELO", 0);
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
    //qDebug() << "ClientPrivate::_k_connected";

    emit q->connected();
}

void ClientPrivate::_k_socketStateChanged(QAbstractSocket::SocketState state)
{
    //qDebug() << "ClientPrivate::_k_socketStateChanged:" << state;

    // register device name, when connected
    if (state == QAbstractSocket::ConnectedState)
        registerName(deviceName);

    if (autoReconnect && connectionRequested
                      && state == QAbstractSocket::UnconnectedState)
        reconnectTimer->start();
    else
        reconnectTimer->stop();

    emit q->stateChanged(mapSocketState(state));
}

void ClientPrivate::_k_socketError(QAbstractSocket::SocketError error)
{
    //qDebug() << "ClientPrivate::_k_socketError:" << error;

    emit q->error(mapSocketError(error));
}

void ClientPrivate::_k_readMessagesFromSocket()
{
    //qDebug() << "ClientPrivate::_k_readMessagesFromSocket";

    // try to read as many messages as possible and stop if the next message
    // cannot be read; emits a messageReceived signal for each message.
    while (readNextMessageFromSocket()) {}
}

void ClientPrivate::_k_autoReconnectTimeout()
{
    //qDebug() << "ClientPrivate::_k_autoReconnectTimeout";

    if (socket->state() == QAbstractSocket::UnconnectedState)
        socket->connectToHost(serverName, serverPort);
}

// ---------------------------------------------------------------------------

/*! \brief Creates a Dcp::Client object.

    The \a parent argument is passed on to the QObject constructor.
 */
Client::Client(QObject *parent)
    : QObject(parent),
      d(new ClientPrivate(this))
{
    connect(d->socket, SIGNAL(connected()), SLOT(_k_connected()),
            Qt::DirectConnection);
    connect(d->socket, SIGNAL(disconnected()), SIGNAL(disconnected()),
            Qt::DirectConnection);
    connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
            SLOT(_k_socketError(QAbstractSocket::SocketError)),
            Qt::DirectConnection);
    connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            SLOT(_k_socketStateChanged(QAbstractSocket::SocketState)),
            Qt::DirectConnection);
    connect(d->socket, SIGNAL(readyRead()), SLOT(_k_readMessagesFromSocket()),
            Qt::DirectConnection);
    connect(d->reconnectTimer, SIGNAL(timeout()), SLOT(_k_autoReconnectTimeout()));
}

/*! \brief Destroys the client, closing the connection if neccessary.
 */
Client::~Client()
{
    delete d;
}

/*! \brief Connects to a DCP server.

    \param serverName the host name or IP address of the DCP server
    \param serverPort the port of the DCP server
    \param deviceName the name that is used to register the device at the
           DCP server

    The \a deviceName must have a maximum length of 16 characters. It is used
    as source or destination in the address fields of DCP messages and must
    be unique. If another device with the same name is already connected to
    the DCP server, any new connection attempt with the same \a deviceName will
    be terminated immediately.

    When a connection is established the connected() signal will be emitted.
    At any time the client can emit error() to signalize that an error
    occurred. If the blocking interface is used (i.e. if no message loop
    exists and the connected() signal can not be handled), you need to call
    waitForConnected() to wait until the connection is established.

    \sa disconnectFromServer(), connected(), waitForConnected()
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

/*! \brief Disconnects from a DCP server.

    Any pending messages will be written to the socket and after that the
    disconnected() signal will be emitted. If the blocking interface is used
    (i.e. if no message loop exists and the disconnected() signal can not be
    handled), you need to call waitForDisconnected() to wait until the
    connection is closed.

    \sa connectToServer(), disconnected(), waitForDisconnected()
 */
void Client::disconnectFromServer()
{
    d->connectionRequested = false;
    d->socket->disconnectFromHost();
}

/*! \brief Returns the next serial number.

    The number returned will be used as serial number for the next message,
    if one of the two overloaded sendMessage() methods are used which do
    not specify a \a snr argument, i.e.:
    - Client::sendMessage(const QByteArray &destination,
                          const QByteArray &data, quint16 flags)
    - Client::sendMessage(const QByteArray &destination,
                          const QByteArray &data, quint8 userFlags,
                          quint8 dcpFlags)

    \sa setNextSnr(), sendMessage()
 */
quint32 Client::nextSnr() const
{
    return d->snr;
}

/*! \brief Sets the next serial number.

    The number will be used as serial number for the next message,
    if one of the two overloaded sendMessage() methods are used which do
    not specify a \a snr argument, i.e.:
    - Client::sendMessage(const QByteArray &destination,
                          const QByteArray &data, quint16 flags)
    - Client::sendMessage(const QByteArray &destination,
                          const QByteArray &data, quint8 userFlags,
                          quint8 dcpFlags)

    \note It is normally not neccessary to adjust the next serial number
          manually. The next serial number will be incremented automatically
          when using the above sendMessage() methods.

    \sa nextSnr(), sendMessage()
 */
void Client::setNextSnr(quint32 snr)
{
    d->snr = snr;
}

/*! \brief Sends a DCP message and handles the serial number automatically.

    \param destination the name of the destination device
    \param data the message data
    \param flags the message flags (combined DCP and user flags)
    \returns the resulting Message object, which is sent to the DCP server

    This method creates a new Message object and sends it to the DCP server.
    The message's serial number is handled by the Client object and the next
    serial number is incremented automatically. The source entry of the
    message is set to the client's deviceName().

    \sa nextSnr(), setNextSnr(), deviceName()
 */
Message Client::sendMessage(const QByteArray &destination,
                            const QByteArray &data, quint16 flags)
{
    Message msg(d->snr, d->deviceName, destination, data, flags);
    d->incrementSnr();
    d->writeMessageToSocket(msg);
    return msg;
}

/*! \brief Sends a DCP message and handles the serial number automatically.

    \param destination the name of the destination device
    \param data the message data
    \param dcpFlags the DCP flags of the message
    \param userFlags the user flags of the message
    \returns the resulting Message object, which is sent to the DCP server

    This method creates a new Message object and sends it to the DCP server.
    The message's serial number is handled by the Client object and the next
    serial number is incremented automatically. The source entry of the
    message is set to the client's deviceName().

    \sa nextSnr(), setNextSnr(), deviceName()
 */
Message Client::sendMessage(const QByteArray &destination,
                            const QByteArray &data, quint8 dcpFlags,
                            quint8 userFlags)
{
    Message msg(d->snr, d->deviceName, destination, data, dcpFlags, userFlags);
    d->incrementSnr();
    d->writeMessageToSocket(msg);
    return msg;
}

/*! \brief Sends a DCP message with an arbitrary serial number.

    \param snr the serial number of the message
    \param destination the name of the destination device
    \param data the message data
    \param flags the message flags (combined DCP and user flags)
    \returns the resulting Message object, which is sent to the DCP server

    This method creates a new Message object and sends it to the DCP server.
    The message's serial number is given by the \a snr argument and does not
    affect the nextSnr() handled by the Client object. The source entry of the
    message is set to the client's deviceName().

    \sa deviceName()
 */
Message Client::sendMessage(quint32 snr, const QByteArray &destination,
                            const QByteArray &data, quint16 flags)
{
    Message msg(snr, d->deviceName, destination, data, flags);
    d->writeMessageToSocket(msg);
    return msg;
}

/*! \brief Sends a DCP message with an arbitrary serial number.

    \param snr the serial number of the message
    \param destination the name of the destination device
    \param data the message data
    \param dcpFlags the DCP flags of the message
    \param userFlags the user flags of the message
    \returns the resulting Message object, which is sent to the DCP server

    This method creates a new Message object and sends it to the DCP server.
    The message's serial number is given by the \a snr argument and does not
    affect the nextSnr() handled by the Client object. The source entry of the
    message is set to the client's deviceName().

    \sa deviceName()
 */
Message Client::sendMessage(quint32 snr, const QByteArray &destination,
                            const QByteArray &data, quint8 dcpFlags,
                            quint8 userFlags)
{
    Message msg(snr, d->deviceName, destination, data, dcpFlags, userFlags);
    d->writeMessageToSocket(msg);
    return msg;
}

/*! \brief Sends a DCP message.

    \param message the message to be sent

    The given Message object is sent as is. The user is responsible to set the
    correct message source; it will not be corrected if it differs from the
    deviceName() of the sending Client object.
 */
void Client::sendMessage(const Message &message)
{
    d->writeMessageToSocket(message);
}

/*! \brief Returns the number of messages that are available for reading.

    \sa readMessage()
 */
int Client::messagesAvailable() const
{
    return d->inQueue.size();
}

/*! \brief Returns the next available message from the input queue.

    This method returns the next unread message and removes it from the
    input queue. If no more unread messages are available a null-message is
    returned.

    \sa messagesAvailable(), Message::isNull()
 */
Message Client::readMessage()
{
    return d->inQueue.isEmpty() ? Message() : d->inQueue.dequeue();
}

/*! \brief Returns the current state of the client.

    This method can be used to check the current state of the client, e.g. if
    the client is connected, about to connect, unconnected or about to close
    the connection.

    \sa isConnected(), stateChanged(), connected(), error()
 */
Client::State Client::state() const
{
    return ClientPrivate::mapSocketState(d->socket->state());
}

/*! \brief Returns true if the client is in the connected state; otherwise
           returns false.

    \sa state(), isUnconnected(), connected()
 */
bool Client::isConnected() const
{
    return d->socket->state() == QAbstractSocket::ConnectedState;
}

/*! \brief Returns true if the client is in the unconnected state; otherwise
           returns false.

    \sa state(), isUnconnected(), connected()
 */
bool Client::isUnconnected() const
{
    return d->socket->state() == QAbstractSocket::UnconnectedState;
}

/*! \brief Returns the type of error that last occurred.

    \sa errorString(), state()
 */
Client::Error Client::error() const
{
    return ClientPrivate::mapSocketError(d->socket->error());
}

/*! \brief Returns a human-readable description of the last error that
           occurred.

    \sa error()
 */
QString Client::errorString() const
{
    return d->socket->errorString();
}

/*! \brief Returns the name of the server as specified by connectToServer(),
           or an empty QString if connectToServer() has not been called yet.

    \sa serverPort(), serverAddress(), localAddress(), deviceName()
 */
QString Client::serverName() const
{
    return d->serverName;
}

/*! \brief Returns the address of the connected server if the client is in
           the ConnectedState; otherwise returns QHostAddress::Null.

    \sa serverName(), serverPort(), localAddress(), deviceName()
 */
QHostAddress Client::serverAddress() const
{
    return d->socket->peerAddress();
}

/*! \brief Returns the server port as specified by connectToServer(),
           or 0 if connectToServer() has not been called yet.

    \sa serverName(), serverAddress(), localPort(), deviceName()
 */
quint16 Client::serverPort() const
{
    return d->serverPort;
}

/*! \brief Returns the device name as specified by connectToServer(),
           or an empty QString if connectToServer() has not been called yet.

    \sa serverName(), serverPort()
 */
QByteArray Client::deviceName() const
{
    return d->deviceName;
}

/*! \brief Returns the host address of the local client socket if available;
           otherwise returns QHostAddress::Null.

    \sa localPort(), serverPort()
 */
QHostAddress Client::localAddress() const
{
    return d->socket->localAddress();
}

/*! \brief Returns the host port of the local client socket if available;
           otherwise returns 0.

    \sa localAddress(), serverPort()
 */
quint16 Client::localPort() const
{
    return d->socket->localPort();
}

/*! \brief Returns true if the auto-reconnect feature is enabled; otherwise
           returns false.

    The auto-reconnect feature is disabled by default.

    \sa setAutoReconnect(), reconnectInterval()
 */
bool Client::autoReconnect() const
{
    return d->autoReconnect;
}

/*! \brief Enables or disables the auto-reconnect feature.

    If the auto-reconnect feature is enabled, the client tries to reconnect
    to the server if a connection attempt failed or the connection was
    terminated by the server. The time between each reconnection attempt
    can be set by using the setReconnectInterval(). If the connection was
    closed manually using the disconnectFromServer() method, no reconnection
    attempt will be performed. By default the auto-reconnect feature is
    disabled and must be enabled explicitly.

    \sa autoReconnect(), setReconnectInterval()
 */
void Client::setAutoReconnect(bool enable)
{
    d->autoReconnect = enable;
    if (enable && d->connectionRequested
               && d->socket->state() == QAbstractSocket::UnconnectedState)
        d->reconnectTimer->start();
    else if (!enable)
        d->reconnectTimer->stop();
}

/*! \brief Returns the auto-reconnect interval in milliseconds.

    The default value of the auto-reconnect interval is 30 seconds.

    \sa setReconnectInterval(), setAutoReconnect()
 */
int Client::reconnectInterval() const
{
    return d->reconnectTimer->interval();
}

/*! \brief Sets the auto-reconnect interval in milliseconds.

    The default value of the auto-reconnect interval is 30 seconds.

    \sa reconnectInterval(), setAutoReconnect()
 */
void Client::setReconnectInterval(int msecs)
{
    d->reconnectTimer->setInterval(msecs);
}

/*! \brief Waits until the client is connected to the server, up to \a msecs
           milliseconds.

    If the connection has been established, this method returns true;
    otherwise it returns false. If the method returns false, you can call
    error() to determine the cause of the error. If \a msecs is -1, this
    method will not time out.

    \note If the method times out, the connection process will be aborted.

    \sa connectToServer(), connected()
 */
bool Client::waitForConnected(int msecs)
{
    // the device name will be registered by the _k_connected() handler
    return d->socket->waitForConnected(msecs);
}

/*! \brief Waits until the client has disconnected from the server, up to
           \a msecs milliseconds.

    If the connection has been disconnected, this method returns true;
    otherwise it returns false. If the method returns false, you can call
    error() to determine the cause of the error. If \a msecs is -1, this
    method will not time out.

    \sa disconnectFromServer(), disconnected()
 */
bool Client::waitForDisconnected(int msecs)
{
    if (d->socket->state() != QAbstractSocket::UnconnectedState)
        return d->socket->waitForDisconnected(msecs);
    return true;
}

/*! \brief Waits until a message is available for reading, up to \a msecs
           milliseconds.

    This method blocks until a message is available for reading, or until
    \a msecs milliseconds have passed. If a message is available, this method
    returns true; otherwise it returns false.  If \a msecs is -1, this
    method will not time out.

    \sa waitForMessagesWritten(), messageReceived()
 */
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

/*! \brief Waits until all pending messages have been sent, up to \a msecs
           milliseconds.

    This method blocks until all messages have been written to the socket,
    or until \a msecs milliseconds have passed. If all messages have been
    written, this method returns true; otherwise it returns false.  If
    \a msecs is -1, this method will not time out.

    \sa waitForReadyRead()
 */
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

#include "client.moc"
