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

#ifndef DCPTIMESERVER_H
#define DCPTIMESERVER_H

#include <dcpclient/client.h>
#include <dcpclient/messageparser.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>

class DcpTimeServer : public QObject
{
    Q_OBJECT

public:
    explicit DcpTimeServer(QObject *parent = 0);
    virtual ~DcpTimeServer();
    void connectToServer(const QString &serverName, quint16 serverPort,
                         const QByteArray &deviceName);

protected slots:
    void error(Dcp::Client::Error error);
    void stateChanged(Dcp::Client::State state);
    void messageReceived();

private:
    Q_DISABLE_COPY(DcpTimeServer)
    Dcp::Client m_dcp;
    Dcp::CommandParser m_parser;
    QByteArray m_timeMode;
};

#endif // DCPTIMESERVER_H
