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

#include <dcpclient.h>
#include <dcpmessage.h>
#include <QtCore/QtCore>

static QTextStream cout(stdout, QIODevice::WriteOnly);
static QTextStream & operator << (QTextStream &os, const Dcp::Message &msg) {
    return os
        << ((msg.flags() & Dcp::Message::PaceFlag) != 0 ? "p" : "-")
        << ((msg.flags() & Dcp::Message::GrecoFlag) != 0 ? "g" : "-")
        << (msg.isUrgent() ? "u" : "-")
        << (msg.isReply() ? "r" : "-")
        << hex << " [0x" << msg.flags() << dec << "] "
        << "#" << msg.snr() << " "
        << msg.source() << " -> "
        << msg.destination() << " "
        << "[" << msg.data().size() << "] "
        << msg.data();
}

int main(int argc, char **argv)
{
    // This app object is only created because of a warning in Qt 4.7,
    // see https://bugreports.qt.nokia.com/browse/QTBUG-14575
    QCoreApplication app(argc, argv);

    Dcp::Client dcp;
    QString serverName = "localhost";
    quint16 serverPort = 2001;
    QByteArray deviceName = "dcplisten";

    cout << "Connecting [" << serverName << ":" << serverPort << "]..."
         << flush;
    dcp.connectToServer(serverName, serverPort, deviceName);
    if (!dcp.waitForConnected()) {
        cout << endl << "Error: " << dcp.errorString() << endl;
        return 1;
    }
    cout << " Connected." << endl;

    // Wait for incomming messags and print them until a message is
    // received that contains the word `quit'.
    cout << "Waiting for incomming message..." << endl;
    while (true)
    {
        if (!dcp.waitForReadyRead(-1)) {
            cout << "Error: " << dcp.errorString() << endl;
            return 1;
        }

        // Read the message and display it.
        Dcp::Message msg = dcp.readMessage();
        cout << msg << endl;

        // leave the loop if the message data contains `quit'.
        if (msg.data().contains("quit"))
            break;
    }

    cout << "Disconnecting from server..." << flush;
    dcp.disconnectFromServer();
    if (!dcp.waitForDisconnected()) {
        cout << endl << "Error: " << dcp.errorString() << endl;
        return 1;
    }
    cout << " Done." << endl;

    return 0;
}
