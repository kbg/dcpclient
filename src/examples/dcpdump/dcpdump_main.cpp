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
#include <QtCore>
#include <csignal>

static QTextStream cout(stdout, QIODevice::WriteOnly);
static QTextStream cerr(stderr, QIODevice::WriteOnly);

static void exitHandler(int param) {
    // shut down the application
    QCoreApplication::exit(0);
}


class CmdLineOptions
{
public:
    CmdLineOptions()
        : serverName("localhost"),
          serverPort(2001),
          deviceName("dcpdump"),
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
            else if (*it == "-p")
            {
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
            else if (*it == "-n")
            {
                if (++it == args.end()) {
                    printReqArg("-n");
                    return false;
                }

                deviceName = it->toLatin1();
            }
            else if (it->startsWith('-'))
            {
                cerr << appName << ": unknown option `" << *it << "'.\n"
                     << moreInfo() << endl;
                return false;
            }
            else
            {
                QStringList devs = it->split(":", QString::SkipEmptyParts);
                if (devs.size() != 2) {
                    cerr << appName << ": invalid device mapping `"
                         << *it << "'.\n" << moreInfo() << endl;
                    return false;
                }

                QByteArray dev1 = devs[0].toLatin1();
                QByteArray dev2 = devs[1].toLatin1();
                if (deviceMap.contains(dev1) || deviceMap.contains(dev2)) {
                    cerr << appName << ": device mappings must be unique.\n"
                         << moreInfo() << endl;
                    return false;
                }

                deviceMap[dev1] = dev2;
                if (dev1 != dev2)
                    deviceMap[dev2] = dev1;
            }
        }

        return true;
    }

    static void printHelp() {
        cout << "Usage: " << qApp->applicationName()
             << " [-s server] [-p port] [-n name] "
             << " [dev1:dev2 [dev3:dev4 [...]]]"
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
    QMap<QByteArray, QByteArray> deviceMap;
    bool help;
};


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QFileInfo(app.arguments()[0]).fileName());

    // use custom signal handler for SIGINT and SIGTERM
    signal(SIGINT, exitHandler);
    signal(SIGTERM, exitHandler);

    CmdLineOptions opts;
    if (!opts.parse()) return 1;
    else if (opts.help) return 0;

    DcpDump dump;
    dump.connectToServer(opts.serverName, opts.serverPort, opts.deviceName);
    dump.setDeviceMap(opts.deviceMap);

    return app.exec();
}
