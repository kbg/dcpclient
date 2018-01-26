/*
 * Copyright (c) 2012 Kolja Glogowski
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

#include "cmdlineoptions.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QString>
#include <QtCore/QStringList>

CmdLineOptions::CmdLineOptions()
    : cout(stdout, QIODevice::WriteOnly),
      cerr(stderr, QIODevice::WriteOnly),
      address(QHostAddress::Any),
      port(2001),
      deviceName("dcphub"),
      debugFlags(DcpHub::NoDebug),
      help(false)
{
}

bool CmdLineOptions::parse()
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
        else if (*it == "-a") {
            if (++it == args.end()) {
                printReqArg("-a");
                return false;
            }

            if (!address.setAddress(*it)) {
                cerr << appName << ": argument of option `-a' must be "
                     << "a valid host address.\n" << moreInfo() << endl;
                return false;
            }
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

            port = quint16(value);
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

            QByteArray value = it->toAscii();
            if (value == "none")
                debugFlags = DcpHub::NoDebug;
            else if (value == "msg")
                debugFlags = DcpHub::MessageDebug;
            else if (value == "pkg")
                debugFlags = DcpHub::PacketDebug;
            else if (value == "full")
                debugFlags = DcpHub::FullDebug;
            else {
                cerr << appName << ": argument of option `-d' must be "
                     << "\"none\", \"msg\", \"pkg\", or \"full\".\n"
                     << moreInfo() << endl;
                return false;
            }
        }
        else if (it->startsWith('-')) {
            cerr << appName << ": unknown option `" << *it << "'.\n"
                 << moreInfo() << endl;
            return false;
        }
        else {
            cerr << appName
                 << ": invalid argument specified.\n"
                 << moreInfo() << endl;
            return false;
        }
    }

    return true;
}

void CmdLineOptions::printHelp()
{
    cout << "Usage: " << qApp->applicationName()
         << " [-a address] [-p port] [-n name] [-d none|msg|pkg|full]"
         << endl;
}

void CmdLineOptions::printReqArg(const QString &optionName)
{
    cerr << qApp->applicationName() << ": option `" << optionName
         << "' requires an argument.\n" << moreInfo() << endl;
}

QString CmdLineOptions::moreInfo()
{
    return QString("Try `%1 --help' for more information.")
            .arg(qApp->applicationName());
}
