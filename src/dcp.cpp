#include "dcp.h"
#include <QByteArray>
#include <QTcpSocket>


DcpMessage::DcpMessage()
{

}

// --------------------------------------------------------------------------

DcpConnection::DcpConnection(QObject *parent)
    : QObject(parent),
      m_socket(new QTcpSocket(this))
{
    connect(m_socket, SIGNAL(connected()), this, SIGNAL(connected()));
    connect(m_socket, SIGNAL(disconnected()), this, SIGNAL(disconnected()));
    connect(m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SIGNAL(error(QAbstractSocket::SocketError)));
    connect(m_socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this, SIGNAL(stateChanged(QAbstractSocket::SocketState)));
}

void DcpConnection::connectToServer(const QString &hostName, quint16 port)
{
    m_socket->connectToHost(hostName, port);
}

void DcpConnection::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

QAbstractSocket::SocketError DcpConnection::error() const
{
    return m_socket->error();
}

QString DcpConnection::errorString() const
{
    return m_socket->errorString();
}

QAbstractSocket::SocketState DcpConnection::state() const
{
    return m_socket->state();
}

bool DcpConnection::waitForConnected(int msecs)
{
    return m_socket->waitForConnected(msecs);
}

bool DcpConnection::waitForDisconnected(int msecs)
{
    return m_socket->waitForDisconnected(msecs);
}

void DcpConnection::sendMessage(const QByteArray &rawData)
{
    m_socket->write(rawData);
}
