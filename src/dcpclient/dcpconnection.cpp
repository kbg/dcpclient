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

#include "dcpconnection.h"
#include "dcpmessage.h"
#include "dcp_p.h"
#include <QtCore/QByteArray>
#include <QtCore/QQueue>
#include <QtCore/QtEndian>
#include <QtCore/QElapsedTimer>
#include <QtNetwork/QTcpSocket>

class DcpConnectionPrivate
{
public:
    DcpConnectionPrivate(DcpConnection *qq) : q(qq) {}

    // private slots
    void _k_readMessagesFromSocket();

    // private data
    DcpConnection * const q;
    QTcpSocket socket;
    QQueue<DcpMessage> inQueue;
};

void DcpConnectionPrivate::_k_readMessagesFromSocket()
{
    // stop if not enough header data is available
    while (socket.bytesAvailable() >= DcpFullHeaderSize)
    {
        char pkgHeader[DcpPacketHeaderSize];
        const quint32 *pMsgSize = reinterpret_cast<const quint32 *>(
                    pkgHeader + DcpPacketMsgSizePos);
        const quint32 *pOffset = reinterpret_cast<const quint32 *>(
                    pkgHeader + DcpPacketOffsetPos);

        socket.peek(pkgHeader, DcpPacketHeaderSize);
        quint32 msgSize = qFromBigEndian(*pMsgSize);
        quint32 offset = qFromBigEndian(*pOffset);

        // not enough data (header + message)
        if (socket.bytesAvailable() < DcpFullHeaderSize + msgSize)
            return;

        // remove packet header from the input buffer
        socket.read(pkgHeader, DcpPacketHeaderSize);

        // read message data
        QByteArray rawMsg = socket.read(DcpMessageHeaderSize + msgSize);

        // ignore multi-packet messages
        if (offset != 0 || rawMsg.size() != DcpMessageHeaderSize + msgSize) {
            qWarning("DcpConnection: Ignoring incoming message. " \
                     "Multi-packet messages are currently not supported.");
        }
        else {
            inQueue.enqueue(DcpMessage::fromRawMsg(rawMsg));
            emit q->readyRead();
        }
    }
}

// ---------------------------------------------------------------------------

DcpConnection::DcpConnection(QObject *parent)
    : QObject(parent),
      d(new DcpConnectionPrivate(this))
{
    connect(&d->socket, SIGNAL(connected()), SIGNAL(connected()));
    connect(&d->socket, SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(&d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                        SIGNAL(error(QAbstractSocket::SocketError)));
    connect(&d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                        SIGNAL(stateChanged(QAbstractSocket::SocketState)));
    connect(&d->socket, SIGNAL(readyRead()), SLOT(_k_readMessagesFromSocket()));
}

DcpConnection::~DcpConnection()
{
    delete d;
}

void DcpConnection::connectToServer(const QString &hostName, quint16 port)
{
    d->socket.connectToHost(hostName, port);
}

void DcpConnection::disconnectFromServer()
{
    d->socket.disconnectFromHost();
}

void DcpConnection::registerName(const QByteArray &deviceName)
{
    DcpMessage msg(0, 0, deviceName, QByteArray(), "HELO");
    writeMessage(msg);
    flush();
}

void DcpConnection::writeMessage(const DcpMessage &msg)
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

    d->socket.write(pkgHeader, DcpPacketHeaderSize);
    d->socket.write(msg.toRawMsg());
}

bool DcpConnection::flush()
{
    return d->socket.flush();
}

int DcpConnection::messagesAvailable() const
{
    return d->inQueue.size();
}

DcpMessage DcpConnection::readMessage()
{
    return d->inQueue.isEmpty() ? DcpMessage() : d->inQueue.dequeue();
}

QAbstractSocket::SocketError DcpConnection::error() const
{
    return d->socket.error();
}

QString DcpConnection::errorString() const
{
    return d->socket.errorString();
}

QAbstractSocket::SocketState DcpConnection::state() const
{
    return d->socket.state();
}

bool DcpConnection::waitForConnected(int msecs)
{
    return d->socket.waitForConnected(msecs);
}

bool DcpConnection::waitForDisconnected(int msecs)
{
    return d->socket.waitForDisconnected(msecs);
}

bool DcpConnection::waitForReadyRead(int msecs)
{
    QElapsedTimer stopWatch;
    stopWatch.start();

    while (messagesAvailable() == 0)
    {
        int msecsLeft = timeoutValue(msecs, stopWatch.elapsed());
        if (!d->socket.waitForReadyRead(msecsLeft))
            return false;

        // try to read messages
        d->_k_readMessagesFromSocket();

        if (msecsLeft == 0)
            break;
    }

    return messagesAvailable() != 0;
}

bool DcpConnection::waitForMessagesWritten(int msecs)
{
    QElapsedTimer stopWatch;
    stopWatch.start();

    while(d->socket.bytesToWrite() != 0)
    {
        int msecsLeft = timeoutValue(msecs, stopWatch.elapsed());
        if (msecsLeft == 0)
            break;

        if (!d->socket.waitForBytesWritten(msecsLeft))
            return false;
    }

    return d->socket.bytesToWrite() == 0;
}

#include "dcpconnection.moc"
