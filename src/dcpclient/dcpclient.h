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

#ifndef DCPCLIENT_H
#define DCPCLIENT_H

#include <QtCore/QObject>

class QByteArray;
class QString;
class DcpMessage;

class DcpMessageParser {
};

class DcpClientPrivate;
class DcpClient : public QObject
{
    Q_OBJECT
    Q_ENUMS(State Error)

public:
    enum State {
        UnconnectedState,
        HostLookupState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };

    enum Error {
        ConnectionRefusedError,
        RemoteHostClosedError,
        HostNotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,
        NetworkError,
        UnsupportedSocketOperationError,
        UnknownSocketError
    };

    explicit DcpClient(QObject *parent = 0);
    virtual ~DcpClient();

    void connectToServer(const QString &serverName, quint16 serverPort,
                         const QByteArray &deviceName);
    void disconnectFromServer();

    /*
    void sendMessage(const QByteArray &destination, const QByteArray &data,
                     quint16 flags = 0);
    void sendMessage(quint32 snr, const QByteArray &destination,
                     const QByteArray &data, quint16 flags = 0);
    */
    void sendMessage(const DcpMessage &message);

    int messagesAvailable() const;
    DcpMessage readMessage();

    DcpClient::State state() const;
    bool isConnected() const;
    bool isUnconnected() const;

    DcpClient::Error error() const;
    QString errorString() const;

    QString serverName() const;
    quint16 serverPort() const;
    QByteArray deviceName() const;

    bool autoReconnect() const;
    void setAutoReconnect(bool enable);
    int reconnectInterval() const;
    void setReconnectInterval(int msecs);

    bool waitForConnected(int msecs = 10000);
    bool waitForDisconnected(int msecs = 10000);
    bool waitForReadyRead(int msecs = 10000);
    bool waitForMessagesWritten(int msecs = 10000);

signals:
    void connected();
    void disconnected();
    void error(DcpClient::Error error);
    void stateChanged(DcpClient::State state);
    void messageReceived();

private:
    Q_PRIVATE_SLOT(d, void _k_connected())
    Q_PRIVATE_SLOT(d, void _k_socketStateChanged(QAbstractSocket::SocketState))
    Q_PRIVATE_SLOT(d, void _k_socketError(QAbstractSocket::SocketError))
    Q_PRIVATE_SLOT(d, void _k_readMessagesFromSocket())
    Q_PRIVATE_SLOT(d, void _k_autoReconnectTimeout())
    Q_DISABLE_COPY(DcpClient)
    friend class DcpClientPrivate;
    DcpClientPrivate * const d;
};

#endif // DCPCLIENT_H
