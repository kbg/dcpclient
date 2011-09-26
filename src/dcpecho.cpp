#include "dcp.h"
#include "echoserver.h"
#include <QtDebug>
#include <QCoreApplication>
#include <csignal>


void exitHandler(int param)
{
    QCoreApplication::exit(0);
}


int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    EchoServer echo;
    echo.connectToServer("localhost");

    signal(SIGINT, exitHandler);
    signal(SIGTERM, exitHandler);
    return app.exec();
}
