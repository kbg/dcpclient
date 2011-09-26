#include "echoserver.h"
#include <QtDebug>
#include <QtCore>


TextInputThread::TextInputThread(QObject *parent)
    : QThread(parent)
{
}

void TextInputThread::run()
{
    QString line;
    QTextStream is(stdin);

    do {
        line = is.readLine();
        emit lineAvailable(line);
    } while (!line.isNull());
}

// ---


EchoServer::EchoServer(QObject *parent)
    : QObject(parent),
      m_dcp(new DcpConnection(this)),
      m_reconnectTimer(new QTimer(this))
{
    connect(m_dcp, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_dcp, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_dcp, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(error(QAbstractSocket::SocketError)));
    connect(m_dcp, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    connect(m_reconnectTimer, SIGNAL(timeout()),
            this, SLOT(reconnectTimer_timeout()));
}

EchoServer::~EchoServer()
{
    qDebug() << "EchoClient::~EchoClient()";
    //if (m_dcp->state() == QAbstractSocket::ConnectedState)
    disconnectFromServer();
}

void EchoServer::connectToServer(const QString &hostName, quint16 port)
{
    m_serverName = hostName;
    m_serverPort = port;
    m_dcp->connectToServer(hostName, port);
}

void EchoServer::disconnectFromServer()
{
    m_dcp->disconnectFromServer();
}

void EchoServer::connected()
{
    qDebug() << "Connected.";

    QByteArray src("echo");

    QByteArray a(55, '\0');
    a[3] = 5;
    a.replace(14, 16, src.leftJustified(16, '\0', true));

    m_dcp->sendMessage(a);
}

void EchoServer::disconnected()
{
    qDebug() << "Disconnected.";
}

void EchoServer::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << "Socket error:" << socketError;
    //qDebug() << "Error:" << m_dcp->errorString();
}

void EchoServer::stateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "Socket state:" << socketState;
    if (m_dcp->state() == QAbstractSocket::UnconnectedState)
        m_reconnectTimer->start(1000);
    else
        m_reconnectTimer->stop();
}

void EchoServer::reconnectTimer_timeout()
{
    if (m_dcp->state() == QAbstractSocket::UnconnectedState)
        m_dcp->connectToServer(m_serverName, m_serverPort);
}
