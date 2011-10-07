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

class MessageValidator {
public:
    virtual bool isValid(const DcpMessage &msg) const = 0;
};

class DcpClientPrivate;
class DcpClient : public QObject
{
    Q_OBJECT

public:
    enum State {
        UnconnectedState,
        HostLookupState,
        ConnectingState,
        ConnectedState,
        //BoundState,  // Servers only
        //ListeningState,  // Servers only
        ClosingState
    };

    enum Error {
        ConnectionRefusedError,
        RemoteHostClosedError,
        HostNotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,
        //DatagramTooLargeError,  // QUdpSocket only
        NetworkError,
        //AddressInUseError,  // QUdpSocket only
        //SocketAddressNotAvailableError,  // QUdpSocket only
        UnsupportedSocketOperationError,
        //UnfinishedSocketOperationError,  // QAbstractSocketEngine only
        //ProxyAuthenticationRequiredError,  // No proxy support
        //SslHandshakeFailedError,  // QSslSocket only
        //ProxyConnectionRefusedError,  // No proxy support
        //ProxyConnectionClosedError,  // No proxy support
        //ProxyConnectionTimeoutError,  // No proxy support
        //ProxyNotFoundError,  // No proxy support
        //ProxyProtocolError,  // No proxy support
        UnknownSocketError = -1
    };

    explicit DcpClient(QObject *parent);
    virtual ~DcpClient();

    void connectToServer(const QByteArray &deviceName,
                         const QString &serverName, quint16 serverPort);
    void disconnectFromServer();

    Error error() const;
    State state() const;

    bool autoAckEnabled() const;
    void setAutoAckEnabled();
    const MessageValidator * autoAckValidator() const;
    void setAutoAckValidator(const MessageValidator *validator);

    bool reconnectEnabled() const;
    void setReconnectEnabled();
    int reconnectInterval() const;
    void setReconnectInterval(int msecs);

signals:
    void connected();
    void disconnected();
    void error(Error error);
    void stateChanged(State state);
    void incomingMessage();

private:
    Q_DISABLE_COPY(DcpClient)
    friend class DcpClientPrivate;
    DcpClientPrivate * const d;
};

#endif // DCPCLIENT_H
