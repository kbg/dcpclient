#include "dcp.h"
#include <QByteArray>
#include <QTcpSocket>
#include <QtEndian>

enum {
    DcpMessageHeaderSize = 42,
    DcpMessageDeviceNameSize = 16,
    DcpMessageFlagsPos = 0,
    DcpMessageSnrPos = 2,
    DcpMessageSourcePos = 6,
    DcpMessageDestinationPos = 22,
    DcpMessageDataLenPos = 38,
};

DcpMessage::DcpMessage()
    : m_valid(true),
      m_flags(0),
      m_snr(0)
{
}

DcpMessage::DcpMessage(quint16 flags, quint32 snr, const QByteArray &source,
                       const QByteArray &destination, const QByteArray &data)
{
    init(flags, snr, source, destination, data);
}

void DcpMessage::init(const QByteArray &rawMsg)
{
    // the message must at least contain the header
    if (rawMsg.size() < DcpMessageHeaderSize) {
        clear();
        m_valid = false;
        return;
    }

    const char *p = rawMsg.constData();

    quint32 dataSize = qFromBigEndian(
        *reinterpret_cast<const quint32 *>(p + DcpMessageDataLenPos));

    // check if message size and data size are consistent
    if (rawMsg.size() != DcpMessageHeaderSize + dataSize) {
        clear();
        m_valid = false;
        return;
    }

    quint16 flags = qFromBigEndian(
        *reinterpret_cast<const quint16 *>(p + DcpMessageFlagsPos));
    quint32 snr = qFromBigEndian(
        *reinterpret_cast<const quint32 *>(p + DcpMessageSnrPos));

    QByteArray source = rawMsg.mid(DcpMessageSourcePos, DcpMessageDeviceNameSize);
    QByteArray destination = rawMsg.mid(
        DcpMessageDestinationPos, DcpMessageDeviceNameSize);
    QByteArray data = rawMsg.right(dataSize);

    // update members
    init(flags, snr, source, destination, data);
}

void DcpMessage::init(quint16 flags, quint32 snr, const QByteArray &source,
                      const QByteArray &destination, const QByteArray &data)
{
    m_valid = true;
    m_flags = flags;
    m_snr = snr;
    m_source = source;
    m_destination = destination;
    m_data = data;
    m_source.truncate(DcpMessageDeviceNameSize);
    m_destination.truncate(DcpMessageDeviceNameSize);
}

void DcpMessage::clear()
{
    m_valid = true;
    m_flags = 0;
    m_snr = 0;
    m_source.clear();
    m_destination.clear();
    m_data.clear();
}

bool DcpMessage::isValid() const
{
    return m_valid;
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
