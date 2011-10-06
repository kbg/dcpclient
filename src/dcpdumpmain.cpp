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

                deviceName = it->toAscii();
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

                QByteArray dev1 = devs[0].toAscii();
                QByteArray dev2 = devs[1].toAscii();
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
    dump.connectToServer(opts.deviceName, opts.serverName, opts.serverPort);
    dump.setDeviceMap(opts.deviceMap);

    return app.exec();
}
