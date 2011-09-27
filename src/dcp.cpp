#include "dcp.h"
#include <QtDebug>
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
    DcpMessageDataLenPos = 38
};


// remove trailing characters
static inline void stripRight(QByteArray &ba, const char c = '\0')
{
    int i = ba.size() - 1;
    for (; i >= 0; --i)
        if (ba.at(i) != c)
            break;
    ba.truncate(i+1);
}


DcpMessage::DcpMessage()
    : m_valid(true),
      m_flags(0),
      m_snr(0)
{
}

DcpMessage::DcpMessage(const DcpMessage &other)
    : m_valid(other.m_valid),
      m_flags(other.m_flags),
      m_snr(other.m_snr),
      m_source(other.m_source),
      m_destination(other.m_destination),
      m_data(other.m_data)
{
}

DcpMessage::DcpMessage(const QByteArray &rawMsg)
{
    init(rawMsg);
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

    QByteArray source = rawMsg.mid(
                DcpMessageSourcePos, DcpMessageDeviceNameSize);
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
    stripRight(m_source);
    m_destination.truncate(DcpMessageDeviceNameSize);
    stripRight(m_destination);
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

quint16 DcpMessage::flags() const
{
    return m_flags;
}

void DcpMessage::setFlags(quint16 flags)
{
    m_valid = true;
    m_flags = flags;
}

quint32 DcpMessage::snr() const
{
    return m_snr;
}

void DcpMessage::setSnr(quint32 snr)
{
    m_valid = true;
    m_snr = snr;
}

QByteArray DcpMessage::source() const
{
    return m_source;
}

void DcpMessage::setSource(const QByteArray &source)
{
    m_valid = true;
    m_source = source;
    m_source.truncate(DcpMessageDeviceNameSize);
}

QByteArray DcpMessage::destination() const
{
    return m_destination;
}

void DcpMessage::setDestination(const QByteArray &destination)
{
    m_valid = true;
    m_destination = destination;
    m_destination.truncate(DcpMessageDeviceNameSize);
}

QByteArray DcpMessage::data() const
{
    return m_data;
}

void DcpMessage::setData(const QByteArray &data)
{
    m_valid = true;
    m_data = data;
}

QByteArray DcpMessage::toRawMsg() const
{
    QByteArray msg = m_data.rightJustified(
                DcpMessageHeaderSize + m_data.size(), '\0', false);

    char *p = msg.data();
    quint16 *pFlags = reinterpret_cast<quint16 *>(p + DcpMessageFlagsPos);
    quint32 *pSnr = reinterpret_cast<quint32 *>(p + DcpMessageSnrPos);
    quint32 *pDataLen = reinterpret_cast<quint32 *>(p + DcpMessageDataLenPos);

    *pFlags = qToBigEndian(m_flags);
    *pSnr = qToBigEndian(m_snr);
    qCopy(m_source.constBegin(), m_source.constEnd(), p + DcpMessageSourcePos);
    qCopy(m_destination.constBegin(), m_destination.constEnd(),
          p + DcpMessageDestinationPos);
    *pDataLen = qToBigEndian(static_cast<quint32>(m_data.size()));
    qCopy(m_data.constBegin(), m_data.constEnd(), p + DcpMessageHeaderSize);

    Q_ASSERT(msg.size() == DcpMessageHeaderSize + m_data.size());
    return msg;
}

DcpMessage DcpMessage::fromRawMsg(const QByteArray &rawMsg)
{
    return DcpMessage(rawMsg);
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
    m_socket->flush();
}

void DcpConnection::sendMessage(const DcpMessage &msg)
{
    char pkgHeader[8];
    *reinterpret_cast<quint32 *>(pkgHeader) = qToBigEndian(
                static_cast<quint32>(msg.data().size()));
    *reinterpret_cast<quint32 *>(pkgHeader+4) = 0;
    m_socket->write(pkgHeader, 8);
    m_socket->write(msg.toRawMsg());
    m_socket->flush();
}
