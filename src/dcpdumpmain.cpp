#include "dcpdump.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <csignal>

static QTextStream qout(stdout, QIODevice::WriteOnly);
static QTextStream qerr(stderr, QIODevice::WriteOnly);

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
          deviceName("dump"),
          help(false)
    {
    }

    bool parse()
    {
        QString appName = qApp->applicationName();
        QString moreInfo = QString("Try `%1' for more information.")
                .arg(appName);

        QStringList args = qApp->arguments();
        for (QStringList::const_iterator it = args.begin()+1;
             it != args.end(); ++it)
        {
            if (*it == "-h" || *it == "--help" || *it == "-help")
            {
                printHelp();
                return true;
            }
            else if (*it == "-s")
            {
                if (++it == args.end()) {
                    qerr << appName << ": option -s requires an argument.\n"
                         << moreInfo << endl;
                    return false;
                }

                serverName = *it;
            }
            else if (*it == "-p")
            {
                if (++it == args.end()) {
                    qerr << appName << ": option -p requires an argument.\n"
                         << moreInfo << endl;
                    return false;
                }

                bool ok;
                ushort value = it->toUShort(&ok);
                if (!ok) {
                    qerr << appName << ": argument of option -p must be "
                         << "an integer.\n" << moreInfo << endl;
                    return false;
                }

                serverPort = quint16(value);
            }
            else if (*it == "-n")
            {
                if (++it == args.end()) {
                    qerr << appName << ": option -n requires an argument.\n"
                         << moreInfo << endl;
                    return false;
                }

                deviceName = it->toAscii();
            }
            else if (it->startsWith('-'))
            {
                qerr << appName << ": unknown option `" << *it << "'.\n"
                     << moreInfo << endl;
                return false;
            }
            else
            {
                QStringList devs = it->split(":", QString::SkipEmptyParts);
                if (devs.size() != 2) {
                    qerr << appName << ": invalid device mapping `"
                         << *it << "'.\n" << moreInfo << endl;
                    return false;
                }

                QByteArray dev1 = devs[0].toAscii();
                QByteArray dev2 = devs[1].toAscii();
                if (deviceMap.contains(dev1) || deviceMap.contains(dev2)) {
                    qerr << appName << ": device mappings must be unique.\n"
                         << moreInfo << endl;
                    return false;
                }

                deviceMap[dev1] = dev2;
                if (dev1 != dev2)
                    deviceMap[dev2] = dev1;
            }
        }

        return true;
    }

    void printHelp() const
    {
        qout << "Usage: " << qApp->applicationName()
             << " [-s server] [-p port] [-n name] "
             << " [dev1:dev2 [dev3:dev4 [...]]]"
             << endl;
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
