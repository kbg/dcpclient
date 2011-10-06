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

#include "dcp.h"
#include <QtCore/QtCore>

static QTextStream cout(stdout, QIODevice::WriteOnly);
static QTextStream cerr(stderr, QIODevice::WriteOnly);
static QTextStream cin(stdin, QIODevice::ReadOnly);
static QTextStream & operator << (QTextStream &os, const DcpMessage &msg) {
    return os
        << (msg.hasPaceFlag() ? "p" : "-")
        << (msg.hasGrecoFlag() ? "g" : "-")
        << (msg.hasUrgentFlag() ? "u" : "-")
        << (msg.hasReplyFlag() ? "r" : "-")
        << hex << " [0x" << msg.flags() << dec << "] "
        << "#" << msg.snr() << " "
        << msg.source() << " -> "
        << msg.destination() << " "
        << "[" << msg.data().size() << "] "
        << msg.data();
}

struct sleep : protected QThread {
    static void s(ulong secs) { QThread::sleep(secs); }
    static void ms(ulong msecs) { QThread::msleep(msecs); }
    static void us(ulong usecs) { QThread::usleep(usecs); }
};

class CmdLineOptions
{
public:
    CmdLineOptions()
        : serverName("localhost"),
          serverPort(2001),
          deviceName("dcpsend"),
          delay(50),
          minConnectionTime(2000),
          verbose(false),
          help(false)
    {
    }

    bool parse()
    {
        QString appName = qApp->applicationName();
        QStringList args = qApp->arguments();
        for (QStringList::const_iterator it = args.begin()+1;
             it != args.end(); ++it)
        {
            if (*it == "-h" || *it == "--help" || *it == "-help") {
                printHelp();
                help = true;
                return true;
            }
            else if (*it == "-s") {
                if (++it == args.end()) {
                    printReqArg("-s");
                    return false;
                }

                serverName = *it;
            }
            else if (*it == "-p") {
                if (++it == args.end()) {
                    printReqArg("-p");
                    return false;
                }

                bool ok;
                ushort value = it->toUShort(&ok);
                if (!ok) {
                    cerr << appName << ": argument of option `-p' must be "
                         << "an integer.\n" << moreInfo() << endl;
                    return false;
                }

                serverPort = quint16(value);
            }
            else if (*it == "-n") {
                if (++it == args.end()) {
                    printReqArg("-n");
                    return false;
                }

                deviceName = it->toAscii();
            }
            else if (*it == "-d") {
                if (++it == args.end()) {
                    printReqArg("-d");
                    return false;
                }

                bool ok;
                int value = it->toInt(&ok);
                if (!ok) {
                    cerr << appName << ": argument of option `-d' must be "
                         << "an integer.\n" << moreInfo() << endl;
                    return false;
                }

                delay = value > 0 ? value : 0;
            }
            else if (*it == "-t") {
                if (++it == args.end()) {
                    printReqArg("-t");
                    return false;
                }

                bool ok;
                int value = it->toInt(&ok);
                if (!ok) {
                    cerr << appName << ": argument of option `-t' must be "
                         << "an integer.\n" << moreInfo() << endl;
                    return false;
                }

                minConnectionTime = value > 0 ? value : 0;
            }
            else if (*it == "-v") {
                verbose = true;
            }
            else if (it->startsWith('-')) {
                cerr << appName << ": unknown option `" << *it << "'.\n"
                     << moreInfo() << endl;
                return false;
            }
            else
                destList << it->toAscii();
        }

        if (destList.isEmpty()) {
            cerr << appName << ": No destination devive specified.\n"
                 << moreInfo() << endl;
            return false;
        }

        return true;
    }

    static void printHelp() {
        cout << "Usage: " << qApp->applicationName()
             << " [-s server] [-p port] [-n name] [-d delay] [-t mintime] [-v] "
             << " dev1 [dev2 [dev3 [...]]]"
             << endl;
    }

    static void printReqArg(const QString &optionName) {
        cerr << qApp->applicationName() << ": option `" << optionName
             << "' requires an argument.\n" << moreInfo() << endl;
    }

    static QString moreInfo() {
        return QString("Try `%1 --help' for more information.")
                .arg(qApp->applicationName());
    }

    QString serverName;
    quint16 serverPort;
    QByteArray deviceName;
    QList<QByteArray> destList;
    int delay;
    int minConnectionTime;
    bool verbose;
    bool help;
};

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QFileInfo(app.arguments()[0]).fileName());

    CmdLineOptions opts;
    if (!opts.parse()) return 1;
    else if (opts.help) return 0;

    // Connect to DCP server
    DcpConnection dcp;
    dcp.connectToServer(opts.serverName, opts.serverPort);
    if (!dcp.waitForConnected()) {
        cerr << "Error: " << dcp.errorString() << endl;
        return 1;
    }

    // Stop the time since the connection was established
    QElapsedTimer stopWatch;
    stopWatch.start();

    dcp.registerName(opts.deviceName);
    if (!dcp.waitForMessagesWritten()) {
        cerr << "Error: " << dcp.errorString() << endl;
        return 1;
    }

    QString line;
    quint32 snr = 0;
    while (true)
    {
        // Read message from stdin
        line = cin.readLine();

        // Stop if stdin is closed
        if (line.isNull())
            break;

        // Ignore empty lines
        if (!line.isEmpty())
        {
            // Send message to all device in destList
            DcpMessage msg(0, 0, opts.deviceName, "", line.toAscii());
            foreach (QByteArray dest, opts.destList)
            {
                msg.setSnr(++snr);
                msg.setDestination(dest);
                dcp.writeMessage(msg);
                if (opts.verbose)
                    cout << msg << endl;
            }

            // Wait until all messages are written
            if (!dcp.waitForMessagesWritten()) {
                cerr << "Error: " << dcp.errorString() << endl;
                return 1;
            }

            // Wait the give time after each line
            sleep::ms(ulong(opts.delay));
        }

        // Read incoming messages
        dcp.waitForReadyRead(1);
        while (dcp.messagesAvailable() > 0) {
            DcpMessage msg = dcp.readMessage();
            if (opts.verbose)
                cout << msg << endl;
        }
    }

    // Wait until the minimum connection time has passed while checking if
    // there are still some remaining incoming messages left
    while (true)
    {
        int timeLeft = int(opts.minConnectionTime - stopWatch.elapsed());

        dcp.waitForReadyRead(timeLeft > 0 ? timeLeft : 0);
        while (dcp.messagesAvailable() > 0) {
            DcpMessage msg = dcp.readMessage();
            if (opts.verbose)
                cout << msg << endl;
        }

        if (opts.minConnectionTime - stopWatch.elapsed() <= 0)
            break;

        sleep::ms(100);
    }

    //cout << stopWatch.elapsed() / 1000. << endl;

    // Disconnect from DCP server
    dcp.disconnectFromServer();
    if (dcp.state() != QAbstractSocket::UnconnectedState
            && !dcp.waitForDisconnected())
    {
        cerr << "Error: " << dcp.errorString() << endl;
        return 1;
    }

    return 0;
}
