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
#include "dcpmessage.h"
#include <QtCore>

static QTextStream cout(stdout, QIODevice::WriteOnly);

DcpDump::DcpDump(QObject *parent)
    : QObject(parent)
{
    m_dcp.setAutoReconnect(true);
    m_dcp.setReconnectInterval(5000);

    connect(&m_dcp, SIGNAL(connected()), this, SLOT(connected()));
    connect(&m_dcp, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(&m_dcp, SIGNAL(error(DcpClient::Error)),
            this, SLOT(error(DcpClient::Error)));
    connect(&m_dcp, SIGNAL(stateChanged(DcpClient::State)),
            this, SLOT(stateChanged(DcpClient::State)));
    connect(&m_dcp, SIGNAL(messageReceived()), this, SLOT(messageReceived()));
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

void DcpDump::error(DcpClient::Error error)
{
    cout << "Error: " << m_dcp.errorString() << "." << endl;
}

void DcpDump::stateChanged(DcpClient::State state)
{
    if (state == DcpClient::ConnectingState)
        cout << "Connecting [" << m_dcp.serverName() << ":"
             << m_dcp.serverPort() << "]..." << endl;
}

void DcpDump::messageReceived()
{
    DcpMessage msg = m_dcp.readMessage();
    QByteArray source = msg.source();
    if (m_deviceMap.contains(source)) {
        msg.setSource(msg.destination());
        msg.setDestination(m_deviceMap[source]);
        m_dcp.sendMessage(msg);
    }

    cout << ((msg.flags() & DcpMessage::PaceFlag) != 0 ? "p" : "-")
         << ((msg.flags() & DcpMessage::GrecoFlag) != 0 ? "g" : "-")
         << (msg.isUrgent() ? "u" : "-")
         << (msg.isReply() ? "r" : "-")
         << hex << " [0x" << msg.flags() << dec << "] "
         << "#" << msg.snr() << " "
         << (m_deviceMap.contains(source) ? (source + " -> ") : "")
         << msg.source() << " -> "
         << msg.destination() << " "
         << "[" << msg.data().size() << "] "
         << msg.data() << endl;
}

#include "dcpdump.moc"
