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
#include "dcp_p.h"
#include <QtCore/QtEndian>
#include <QtCore/QtAlgorithms>

/*!
    Default constructor.

    Creates a Null-Message.

    \sa isNull()
 */
DcpMessage::DcpMessage()
    : m_null(true),
      m_flags(0),
      m_snr(0)
{
}

/*!
    Copy constructor.
 */
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

bool DcpMessage::hasPaceFlag() const
{
    return (m_flags & PaceFlag) != 0;
}

bool DcpMessage::hasGrecoFlag() const
{
    return (m_flags & GrecoFlag) != 0;
}

bool DcpMessage::hasUrgentFlag() const
{
    return (m_flags & UrgentFlag) != 0;
}

bool DcpMessage::hasReplyFlag() const
{
    return (m_flags & ReplyFlag) != 0;
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
