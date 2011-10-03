#include "dcp.h"
#include "QCoreApplication"
#include <QTextStream>

static QTextStream cout(stdout, QIODevice::WriteOnly);
static QTextStream & operator << (QTextStream &os, const DcpMessage &msg) {
    return os
       << hex << "0x" << msg.flags() << dec << " "
       << "#" << msg.snr() << " "
       << msg.source() << " -> "
       << msg.destination() << " "
       << "[" << msg.data().size() << "] "
       << msg.data();
}

int main(int argc, char **argv)
{
    // This app object is only created because of a warning in
    // Qt 4.7, see https://bugreports.qt.nokia.com/browse/QTBUG-14575
    QCoreApplication app(argc, argv);

    DcpConnection dcp;
    QString hostName = "localhost";
    quint16 port = 2001;
    QByteArray deviceName = "listener";

    cout << "Connecting [" << hostName << ":" << port << "]..." << flush;
    dcp.connectToServer(hostName, port);
    if (!dcp.waitForConnected()) {
        cout << endl << "Error: " << dcp.errorString() << endl;
        return 1;
    }
    cout << " Connected." << endl;

    // Tell the server who we are.
    cout << "Registering device name [" << deviceName << "]..." << flush;
    dcp.registerName(deviceName);
    if (!dcp.waitForMessagesWritten()) {
        cout << endl << "Error: " << dcp.errorString() << endl;
        return 1;
    }
    cout << " Done." << endl;

    // Wait for incomming messags and print them until a message ist
    // received that contains the word `quit'.
    cout << "Waiting for incomming message..." << endl;
    while (true)
    {
        if (!dcp.waitForReadyRead(-1)) {
            cout << "Error: " << dcp.errorString() << endl;
            return 1;
        }

        // Read the message and display it.
        DcpMessage msg = dcp.readMessage();
        cout << msg << endl;

        // leave the loop if the message data contains `quit'.
        if (msg.data().contains("quit"))
            break;
    }

    cout << "Disconnecting from server..." << flush;
    dcp.disconnectFromServer();
    if (dcp.state() != QAbstractSocket::UnconnectedState
            && !dcp.waitForDisconnected())
    {
        cout << endl << "Error: " << dcp.errorString() << endl;
        return 1;
    }
    cout << " Done." << endl;

    return 0;
}
