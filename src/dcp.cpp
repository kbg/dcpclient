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

enum {
    DcpPacketHeaderSize = 8,
    DcpPacketMsgSizePos = 0,
    DcpPacketOffsetPos = 4
};

enum {
    DcpFullHeaderSize = DcpMessageHeaderSize + DcpPacketHeaderSize,
    DcpMaxPacketSize = 0x10000
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
    : m_null(true),
      m_flags(0),
      m_snr(0)
{
}

DcpMessage::DcpMessage(const DcpMessage &other)
    : m_null(other.m_null),
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
        return;
    }

    const char *p = rawMsg.constData();

    quint32 dataSize = qFromBigEndian(
                *reinterpret_cast<const quint32 *>(p + DcpMessageDataLenPos));

    // check if message size and data size are consistent
    if (rawMsg.size() != DcpMessageHeaderSize + dataSize) {
        clear();
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
    m_null = false;
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
    m_null = true;
    m_flags = 0;
    m_snr = 0;
    m_source.clear();
    m_destination.clear();
    m_data.clear();
}

bool DcpMessage::isNull() const
{
    return m_null;
}

quint16 DcpMessage::flags() const
{
    return m_flags;
}

void DcpMessage::setFlags(quint16 flags)
{
    m_null = false;
    m_flags = flags;
}

quint32 DcpMessage::snr() const
{
    return m_snr;
}

void DcpMessage::setSnr(quint32 snr)
{
    m_null = false;
    m_snr = snr;
}

QByteArray DcpMessage::source() const
{
    return m_source;
}

void DcpMessage::setSource(const QByteArray &source)
{
    m_null = false;
    m_source = source;
    m_source.truncate(DcpMessageDeviceNameSize);
}

QByteArray DcpMessage::destination() const
{
    return m_destination;
}

void DcpMessage::setDestination(const QByteArray &destination)
{
    m_null = false;
    m_destination = destination;
    m_destination.truncate(DcpMessageDeviceNameSize);
}

QByteArray DcpMessage::data() const
{
    return m_data;
}

void DcpMessage::setData(const QByteArray &data)
{
    m_null = false;
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
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
}

void DcpConnection::connectToServer(const QString &hostName, quint16 port)
{
    m_socket->connectToHost(hostName, port);
}

void DcpConnection::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void DcpConnection::registerName(const QByteArray &deviceName)
{
    DcpMessage msg(0, 0, deviceName, QByteArray(), "HELO");
    writeMessage(msg);
}

void DcpConnection::writeMessage(const DcpMessage &msg)
{
    if (msg.isNull()) {
        qWarning("DcpConnection::sendMessage: Ignoring invalid message.");
        return;
    }

    if (!msg.data().size() + DcpFullHeaderSize > DcpMaxPacketSize) {
        qWarning("DcpConnection::sendMessage: Skipping large message. " \
                 "Multi-packet messages are currently not supported.");
        return;
    }

    char pkgHeader[DcpPacketHeaderSize];
    quint32 *pMsgSize = reinterpret_cast<quint32 *>(
                pkgHeader + DcpPacketMsgSizePos);
    quint32 *pOffset = reinterpret_cast<quint32 *>(
                pkgHeader + DcpPacketOffsetPos);

    *pMsgSize = qToBigEndian(static_cast<quint32>(msg.data().size()));
    *pOffset = 0;

    m_socket->write(pkgHeader, DcpPacketHeaderSize);
    m_socket->write(msg.toRawMsg());
}

bool DcpConnection::flush()
{
    return m_socket->flush();
}

int DcpConnection::messagesAvailable() const
{
    return m_inQueue.size();
}

DcpMessage DcpConnection::readMessage()
{
    return m_inQueue.isEmpty() ? DcpMessage() : m_inQueue.dequeue();
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

void DcpConnection::onSocketReadyRead()
{
    // stop if not enough header data is available
    while (m_socket->bytesAvailable() >= DcpFullHeaderSize)
    {
        char pkgHeader[DcpPacketHeaderSize];
        const quint32 *pMsgSize = reinterpret_cast<const quint32 *>(
                    pkgHeader + DcpPacketMsgSizePos);
        const quint32 *pOffset = reinterpret_cast<const quint32 *>(
                    pkgHeader + DcpPacketOffsetPos);

        m_socket->peek(pkgHeader, DcpPacketHeaderSize);
        quint32 msgSize = qFromBigEndian(*pMsgSize);
        quint32 offset = qFromBigEndian(*pOffset);

        // not enough data (header + message)
        if (m_socket->bytesAvailable() < DcpFullHeaderSize + msgSize)
            return;

        // remove packet header from the input buffer
        m_socket->read(pkgHeader, DcpPacketHeaderSize);

        // read message data
        QByteArray rawMsg = m_socket->read(DcpMessageHeaderSize + msgSize);

        // ignore multi-packet messages
        if (offset != 0 || rawMsg.size() != DcpMessageHeaderSize + msgSize) {
            qWarning("DcpConnection: Ignoring incoming message. " \
                     "Multi-packet messages are currently not supported.");
        }
        else {
            m_inQueue.enqueue(DcpMessage::fromRawMsg(rawMsg));
            emit readyRead();
        }
    }
}
