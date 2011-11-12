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

#include "dcptime.h"
#include <dcpmessage.h>
#include <dcpmessageparser.h>
#include <QtCore>
using namespace Dcp;

static QTextStream cout(stdout, QIODevice::WriteOnly);

DcpTime::DcpTime(QObject *parent)
    : QObject(parent)
{
    m_dcp.setAutoReconnect(true);
    connect(&m_dcp, SIGNAL(error(Dcp::DcpClient::Error)),
            this, SLOT(error(Dcp::DcpClient::Error)));
    connect(&m_dcp, SIGNAL(stateChanged(Dcp::DcpClient::State)),
            this, SLOT(stateChanged(Dcp::DcpClient::State)));
    connect(&m_dcp, SIGNAL(messageReceived()), this, SLOT(messageReceived()));
}

DcpTime::~DcpTime()
{
    m_dcp.disconnectFromServer();
}

void DcpTime::connectToServer(const QString &serverName, quint16 serverPort,
                              const QByteArray &deviceName)
{
    m_dcp.connectToServer(serverName, serverPort, deviceName);
}

void DcpTime::error(DcpClient::Error error)
{
    cout << "Error: " << m_dcp.errorString() << "." << endl;
}

void DcpTime::stateChanged(DcpClient::State state)
{
    switch (state)
    {
    case DcpClient::ConnectingState:
        cout << "Connecting [" << m_dcp.serverName() << ":"
             << m_dcp.serverPort() << "]..." << endl;
        break;
    case DcpClient::ConnectedState:
        cout << "Connected [" << m_dcp.deviceName() << "]." << endl;
        break;
    case DcpClient::UnconnectedState:
        cout << "Disconnected." << endl;
        break;
    default:
        break;
    }
}

void DcpTime::messageReceived()
{
    DcpMessage msg = m_dcp.readMessage();

    // ignore reply messages
    if (msg.isReply())
        return;

    if (!m_parser.parse(msg)) {
        cout << "Error parsing dcp message." << endl;
        return;
    }

    QDateTime now = QDateTime::currentDateTimeUtc();
    QList<QByteArray> args = m_parser.arguments();
    switch (m_parser.commandType())
    {
    case CommandParser::GetCommand:
        if (args.isEmpty())
            m_dcp.sendMessage(msg.ackMessage(AckUnknowCommandError));
        else if (args.size() != 1)
            m_dcp.sendMessage(msg.ackMessage(AckParameterError));
        else if (args[0] == "time") {
            m_dcp.sendMessage(msg.ackMessage());
            m_dcp.sendMessage(msg.replyMessage(
                                  now.toString("HH:mm:ss.zzz").toAscii()));
        }
        else
        break;
    case CommandParser::SetCommand:
        break;
    default:
        m_dcp.sendMessage(msg.ackMessage(AckUnknowCommandError));
    }
}

#include "dcptime.moc"
