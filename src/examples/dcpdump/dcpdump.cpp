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

#include "dcpdump.h"
#include <dcpclient/message.h>
#include <QtCore>

static QTextStream cout(stdout, QIODevice::WriteOnly);

DcpDump::DcpDump(QObject *parent)
    : QObject(parent)
{
    m_dcp.setAutoReconnect(true);
    m_dcp.setReconnectInterval(5000);

    connect(&m_dcp, SIGNAL(connected()), SLOT(connected()));
    connect(&m_dcp, SIGNAL(disconnected()), SLOT(disconnected()));
    connect(&m_dcp, SIGNAL(error(Dcp::Client::Error)),
                    SLOT(error(Dcp::Client::Error)));
    connect(&m_dcp, SIGNAL(stateChanged(Dcp::Client::State)),
                    SLOT(stateChanged(Dcp::Client::State)));
    connect(&m_dcp, SIGNAL(messageReceived()), SLOT(messageReceived()));
}

DcpDump::~DcpDump()
{
    disconnectFromServer();
}

void DcpDump::connectToServer(const QString &serverName, quint16 serverPort,
                              const QByteArray &deviceName)
{
    m_dcp.connectToServer(serverName, serverPort, deviceName);
}

void DcpDump::disconnectFromServer()
{
    m_dcp.disconnectFromServer();
}

void DcpDump::setDeviceMap(const QMap<QByteArray, QByteArray> &deviceMap)
{
    m_deviceMap = deviceMap;
}

void DcpDump::connected()
{
    cout << "Connected [" << m_dcp.deviceName() << "]." << endl;
}

void DcpDump::disconnected()
{
    cout << "Disconnected." << endl;
}

void DcpDump::error(Dcp::Client::Error error)
{
    cout << "Error: " << m_dcp.errorString() << "." << endl;
}

void DcpDump::stateChanged(Dcp::Client::State state)
{
    if (state == Dcp::Client::ConnectingState)
        cout << "Connecting [" << m_dcp.serverName() << ":"
             << m_dcp.serverPort() << "]..." << endl;
}

void DcpDump::messageReceived()
{
    Dcp::Message msg = m_dcp.readMessage();
    QByteArray source = msg.source();
    if (m_deviceMap.contains(source)) {
        msg.setSource(msg.destination());
        msg.setDestination(m_deviceMap[source]);
        m_dcp.sendMessage(msg);
    }

    cout << msg << endl;
}
