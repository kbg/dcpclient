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

#include "dcptimeserver.h"
#include <dcpclient/message.h>
#include <QtCore/QtCore>

static QTextStream cout(stdout, QIODevice::WriteOnly);

DcpTimeServer::DcpTimeServer(QObject *parent)
    : QObject(parent), m_timeMode("utc")
{
    m_dcp.setAutoReconnect(true);
    connect(&m_dcp, SIGNAL(error(Dcp::Client::Error)),
                    SLOT(error(Dcp::Client::Error)));
    connect(&m_dcp, SIGNAL(stateChanged(Dcp::Client::State)),
                    SLOT(stateChanged(Dcp::Client::State)));
    connect(&m_dcp, SIGNAL(messageReceived()), SLOT(messageReceived()));
}

DcpTimeServer::~DcpTimeServer()
{
    m_dcp.disconnectFromServer();
}

void DcpTimeServer::connectToServer(const QString &serverName,
                                    quint16 serverPort,
                                    const QByteArray &deviceName)
{
    m_dcp.connectToServer(serverName, serverPort, deviceName);
}

void DcpTimeServer::error(Dcp::Client::Error error)
{
    cout << "Error: " << m_dcp.errorString() << "." << endl;
}

void DcpTimeServer::stateChanged(Dcp::Client::State state)
{
    switch (state)
    {
    case Dcp::Client::ConnectingState:
        cout << "Connecting [" << m_dcp.serverName() << ":"
             << m_dcp.serverPort() << "]..." << endl;
        break;
    case Dcp::Client::ConnectedState:
        cout << "Connected [" << m_dcp.deviceName() << "]." << endl;
        break;
    case Dcp::Client::UnconnectedState:
        cout << "Disconnected." << endl;
        break;
    default:
        break;
    }
}

void DcpTimeServer::messageReceived()
{
    Dcp::Message msg = m_dcp.readMessage();

    // ignore reply messages
    if (msg.isReply())
        return;

    // parse command messages
    if (!m_parser.parse(msg)) {
        m_dcp.sendMessage(msg.ackMessage(Dcp::AckUnknownCommandError));
        return;
    }

    // get current date and time
    QDateTime now = (m_timeMode == "local") ?
        QDateTime::currentDateTime() : QDateTime::currentDateTimeUtc();

    // handle command messages
    QList<QByteArray> args = m_parser.arguments();
    QByteArray identifier = m_parser.identifier();
    static const QList<QByteArray> getters = QList<QByteArray>()
            << "mode" << "time" << "date" << "datetime" << "julian";
    switch (m_parser.commandType())
    {
    case Dcp::CommandParser::GetCommand:
        // check for valid identifier
        if (!getters.contains(identifier)) {
            m_dcp.sendMessage(msg.ackMessage(Dcp::AckUnknownCommandError));
            return;
        }

        // all supported get commands have no additional argument
        if (!args.isEmpty()) {
            m_dcp.sendMessage(msg.ackMessage(Dcp::AckParameterError));
            return;
        }

        // command is valid
        m_dcp.sendMessage(msg.ackMessage());

        if (identifier == "mode") {
            m_dcp.sendMessage(msg.replyMessage(m_timeMode));
        }
        else if (identifier == "time") {
            m_dcp.sendMessage(msg.replyMessage(
                now.toString("HH:mm:ss.zzz").toAscii()));
        }
        else if (identifier == "date") {
            m_dcp.sendMessage(msg.replyMessage(
                now.toString("yyyy-MM-dd").toAscii()));
        }
        else if (identifier == "datetime") {
            m_dcp.sendMessage(msg.replyMessage(
                now.toString("yyyy-MM-ddTHH:mm:ss.zzz").toAscii()));
        }
        else if (identifier == "julian") {
            now = now.toUTC();
            double julian = now.date().toJulianDay() - 0.5;
            julian += now.time().hour() / 24.0;
            julian += now.time().minute() / (24.0 * 60.0);
            julian += now.time().second() / (24.0 * 3600.0);
            julian += now.time().msec() / (24.0 * 3600000.0);
            m_dcp.sendMessage(msg.replyMessage(
                QByteArray::number(julian, 'f', 8)));
        }
        break;

    case Dcp::CommandParser::SetCommand:
        // only set mode command with one argument
        if (identifier != "mode") {
            m_dcp.sendMessage(msg.ackMessage(Dcp::AckUnknownCommandError));
        }
        else if (args.size() != 1 || (args[0] != "local" && args[0] != "utc")) {
            m_dcp.sendMessage(msg.ackMessage(Dcp::AckParameterError));
        }
        else {
            m_dcp.sendMessage(msg.ackMessage());
            m_timeMode = args[0];
            m_dcp.sendMessage(msg.replyMessage("FIN"));
        }
        break;

    case Dcp::CommandParser::DefCommand:
    case Dcp::CommandParser::UndefCommand:
        // there are no supported def or undef commands
        m_dcp.sendMessage(msg.ackMessage(Dcp::AckUnknownCommandError));
        break;
    }
}

#include "dcptimeserver.moc"
