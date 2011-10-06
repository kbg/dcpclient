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
