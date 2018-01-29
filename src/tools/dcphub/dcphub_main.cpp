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

#include "dcphub.h"
#include "cmdlineoptions.h"
#include <QtCore>
#include <csignal>

static void exitHandler(int param) {
    // shut down the application
    QTextStream cout(stdout, QIODevice::WriteOnly);
    cout << "Shutting down..." << endl;
    QCoreApplication::exit(0);
}

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

    DcpHub dcpHub;
    dcpHub.setDeviceName(opts.deviceName);
    dcpHub.setDebugFlags(opts.debugFlags);
    if (!dcpHub.listen(opts.address, opts.port))
        return 1;

    return app.exec();
}
