/*
 * Copyright (c) 2011 Kolja Glogowski
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

#include "dcpmessage.h"
#include "dcpclient_p.h"
#include <QtCore/QtEndian>
#include <QtCore/QtAlgorithms>

namespace Dcp {

/*!
    Default constructor.

    Creates a Null-Message.

    \sa isNull()
 */
Message::Message()
    : m_null(true),
      m_flags(0),
      m_snr(0)
{
}

/*!
    Copy constructor.
 */
Message::Message(const Message &other)
    : m_null(other.m_null),
      m_flags(other.m_flags),
      m_snr(other.m_snr),
      m_source(other.m_source),
      m_destination(other.m_destination),
      m_data(other.m_data)
{
}

Message::Message(const QByteArray &rawMsg)
{
    init(rawMsg);
}

Message::Message(quint16 flags, quint32 snr, const QByteArray &source,
                 const QByteArray &destination, const QByteArray &data)
{
    init(flags, snr, source, destination, data);
}

void Message::init(const QByteArray &rawMsg)
{
    // the message must at least contain the header
    if (rawMsg.size() < MessageHeaderSize) {
        clear();
        return;
    }

    const char *p = rawMsg.constData();

    quint32 dataSize = qFromBigEndian(
                *reinterpret_cast<const quint32 *>(p + MessageDataLenPos));

    // check if message size and data size are consistent
    if (rawMsg.size() != int(MessageHeaderSize + dataSize)) {
        clear();
        return;
    }

    quint16 flags = qFromBigEndian(
                *reinterpret_cast<const quint16 *>(p + MessageFlagsPos));
    quint32 snr = qFromBigEndian(
                *reinterpret_cast<const quint32 *>(p + MessageSnrPos));

    QByteArray source = rawMsg.mid(MessageSourcePos, MessageDeviceNameSize);
    QByteArray destination = rawMsg.mid(
                MessageDestinationPos, MessageDeviceNameSize);
    QByteArray data = rawMsg.right(dataSize);

    // update members
    init(flags, snr, source, destination, data);
}

void Message::init(quint16 flags, quint32 snr, const QByteArray &source,
                   const QByteArray &destination, const QByteArray &data)
{
    m_null = false;
    m_flags = flags;
    m_snr = snr;
    m_source = source;
    m_destination = destination;
    m_data = data;
    m_source.truncate(MessageDeviceNameSize);
    stripRight(m_source);
    m_destination.truncate(MessageDeviceNameSize);
    stripRight(m_destination);
}

void Message::clear()
{
    m_null = true;
    m_flags = 0;
    m_snr = 0;
    m_source.clear();
    m_destination.clear();
    m_data.clear();
}

bool Message::isNull() const
{
    return m_null;
}

quint16 Message::flags() const
{
    return m_flags;
}

void Message::setFlags(quint16 flags)
{
    m_null = false;
    m_flags = flags;
}

quint8 Message::dcpFlags() const
{
    return quint8(m_flags & 0x00ff);
}

void Message::setDcpFlags(quint8 flags)
{
    m_null = false;
    m_flags &= 0xff00;
    m_flags |= quint16(flags);
}

quint8 Message::userFlags() const
{
    return quint8(m_flags >> 8);
}

void Message::setUserFlags(quint8 flags)
{
    m_null = false;
    m_flags &= 0x00ff;
    m_flags |= quint16(flags) << 8;
}

bool Message::isUrgent() const
{
    return (m_flags & UrgentFlag) != 0;
}

bool Message::isReply() const
{
    return (m_flags & ReplyFlag) != 0;
}

quint32 Message::snr() const
{
    return m_snr;
}

void Message::setSnr(quint32 snr)
{
    m_null = false;
    m_snr = snr;
}

QByteArray Message::source() const
{
    return m_source;
}

void Message::setSource(const QByteArray &source)
{
    m_null = false;
    m_source = source;
    m_source.truncate(MessageDeviceNameSize);
}

QByteArray Message::destination() const
{
    return m_destination;
}

void Message::setDestination(const QByteArray &destination)
{
    m_null = false;
    m_destination = destination;
    m_destination.truncate(MessageDeviceNameSize);
}

QByteArray Message::data() const
{
    return m_data;
}

void Message::setData(const QByteArray &data)
{
    m_null = false;
    m_data = data;
}

QByteArray Message::toRawMsg() const
{
    QByteArray msg = m_data.rightJustified(
                MessageHeaderSize + m_data.size(), '\0', false);

    char *p = msg.data();
    quint16 *pFlags = reinterpret_cast<quint16 *>(p + MessageFlagsPos);
    quint32 *pSnr = reinterpret_cast<quint32 *>(p + MessageSnrPos);
    quint32 *pDataLen = reinterpret_cast<quint32 *>(p + MessageDataLenPos);

    *pFlags = qToBigEndian(m_flags);
    *pSnr = qToBigEndian(m_snr);
    qCopy(m_source.constBegin(), m_source.constEnd(), p + MessageSourcePos);
    qCopy(m_destination.constBegin(), m_destination.constEnd(),
          p + MessageDestinationPos);
    *pDataLen = qToBigEndian(static_cast<quint32>(m_data.size()));
    qCopy(m_data.constBegin(), m_data.constEnd(), p + MessageHeaderSize);

    Q_ASSERT(msg.size() == MessageHeaderSize + m_data.size());
    return msg;
}

Message Message::fromRawMsg(const QByteArray &rawMsg)
{
    return Message(rawMsg);
}

Message Message::ackMessage(int errorCode) const
{
    return Message(
        m_flags | ReplyFlag | UrgentFlag,
        m_snr, m_destination, m_source,
        QByteArray::number(errorCode) + " ACK");
}

Message Message::replyMessage(const QByteArray &data, int errorCode) const
{
    return Message(
        m_flags | ReplyFlag,
        m_snr, m_destination, m_source,
        QByteArray::number(errorCode) + " " + (data.isEmpty() ? "FIN" : data));
}

} // namespace Dcp
