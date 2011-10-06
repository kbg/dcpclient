#include "dcpdump.h"
#include "dcp.h"
#include <QtCore>

static QTextStream cout(stdout, QIODevice::WriteOnly);

DcpDump::DcpDump(QObject *parent)
    : QObject(parent),
      m_dcp(new DcpConnection(this)),
      m_reconnectTimer(new QTimer(this)),
      m_reconnect(false)
{
    m_reconnectTimer->setInterval(3000);
    connect(m_dcp, SIGNAL(connected()), this, SLOT(connected()));
    connect(m_dcp, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_dcp, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(error(QAbstractSocket::SocketError)));
    connect(m_dcp, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    connect(m_dcp, SIGNAL(readyRead()), this, SLOT(messageReady()));
    connect(m_reconnectTimer, SIGNAL(timeout()),
            this, SLOT(reconnectTimer_timeout()));
}

DcpDump::~DcpDump()
{
    disconnectFromServer();
}

void DcpDump::connectToServer(const QByteArray &deviceName,
                              const QString &serverName, quint16 serverPort)
{
    m_reconnect = true;
    m_deviceName = deviceName;
    m_serverName = serverName;
    m_serverPort = serverPort;
    m_dcp->connectToServer(serverName, serverPort);
}

void DcpDump::disconnectFromServer()
{
    m_reconnect = false;
    m_reconnectTimer->stop();
    m_dcp->disconnectFromServer();
}

void DcpDump::setReconnectInterval(int msec)
{
    m_reconnectTimer->setInterval(msec);
}

void DcpDump::setDeviceMap(const QMap<QByteArray, QByteArray> &deviceMap)
{
    m_deviceMap = deviceMap;
}

void DcpDump::connected()
{
    cout << "Connected [" << m_deviceName << "]." << endl;
    m_dcp->registerName(m_deviceName);
}

void DcpDump::disconnected()
{
    cout << "Disconnected." << endl;
}

void DcpDump::error(QAbstractSocket::SocketError socketError)
{
    cout << m_dcp->errorString() << "." << endl;
}

void DcpDump::stateChanged(QAbstractSocket::SocketState socketState)
{
    //qDebug() << socketState;
    switch (m_dcp->state())
    {
    case QAbstractSocket::UnconnectedState:
        if (m_reconnect)
            m_reconnectTimer->start();
        break;
    case QAbstractSocket::ConnectingState:
        cout << "Connecting [" << m_serverName << ":" << m_serverPort
                << "]..." << endl;
    default:
        m_reconnectTimer->stop();
    }
}

void DcpDump::messageReady()
{
    DcpMessage msg = m_dcp->readMessage();
    QByteArray source = msg.source();
    if (m_deviceMap.contains(source)) {
        msg.setSource(msg.destination());
        msg.setDestination(m_deviceMap[source]);
        m_dcp->writeMessage(msg);
    }

    cout << (msg.hasPaceFlag() ? "p" : "-")
         << (msg.hasGrecoFlag() ? "g" : "-")
         << (msg.hasUrgentFlag() ? "u" : "-")
         << (msg.hasReplyFlag() ? "r" : "-")
         << hex << " [0x" << msg.flags() << dec << "] "
         << "#" << msg.snr() << " "
         << (m_deviceMap.contains(source) ? (source + " -> ") : "")
         << msg.source() << " -> "
         << msg.destination() << " "
         << "[" << msg.data().size() << "] "
         << msg.data() << endl;
}

void DcpDump::reconnectTimer_timeout()
{
    if (m_dcp->state() == QAbstractSocket::UnconnectedState)
        m_dcp->connectToServer(m_serverName, m_serverPort);
}
