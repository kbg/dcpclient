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
#include <QtCore/QSharedData>
#include <QtCore/QtEndian>
#include <QtCore/QtAlgorithms>

namespace Dcp {

/*! \internal
    \brief Implicitly shared message data.
    \todo Use a static null object instead of the isNull flag. This needs to
          be carefully implemented to keep the Message class reentrant.
 */
class MessageData : public QSharedData
{
public:
    MessageData() : isNull(true), flags(0), snr(0) {}
    MessageData(const MessageData &other) : QSharedData(other),
            isNull(other.isNull), flags(other.flags), snr(other.snr) {}
    MessageData(quint16 flags_, quint32 snr_, const QByteArray &source_,
            const QByteArray &destination_, const QByteArray &data_);

    bool isNull;
    quint16 flags;
    quint32 snr;
    QByteArray source;
    QByteArray destination;
    QByteArray data;
};

MessageData::MessageData(quint16 flags_, quint32 snr_,
        const QByteArray &source_, const QByteArray &destination_,
        const QByteArray &data_)
    : isNull(false),
      flags(flags_),
      snr(snr_),
      source(source_),
      destination(destination_),
      data(data_)
{
    source.truncate(MessageDeviceNameSize);
    stripRight(source);
    destination.truncate(MessageDeviceNameSize);
    stripRight(destination);
}

// -------------------------------------------------------------------------

/*!
    Default constructor.

    Creates a Null-Message.

    \sa isNull()
 */
Message::Message()
    : d(new MessageData)
{
}

/*!
    Copy constructor.
 */
Message::Message(const Message &other)
    : d(other.d)
{
}

Message::Message(quint16 flags, quint32 snr, const QByteArray &source,
                 const QByteArray &destination, const QByteArray &data)
    : d(new MessageData(flags, snr, source, destination, data))
{
}

Message::~Message()
{
}

Message & Message::operator=(const Message &other)
{
    d = other.d;
    return *this;
}

void Message::clear()
{
    d->isNull = true;
    d->flags = 0;
    d->snr = 0;
    d->source.clear();
    d->destination.clear();
    d->data.clear();
}

bool Message::isNull() const
{
    return d->isNull;
}

quint16 Message::flags() const
{
    return d->flags;
}

void Message::setFlags(quint16 flags)
{
    d->isNull = false;
    d->flags = flags;
}

quint8 Message::dcpFlags() const
{
    return quint8(d->flags & 0x00ff);
}

void Message::setDcpFlags(quint8 flags)
{
    d->isNull = false;
    d->flags &= 0xff00;
    d->flags |= quint16(flags);
}

quint8 Message::userFlags() const
{
    return quint8(d->flags >> 8);
}

void Message::setUserFlags(quint8 flags)
{
    d->isNull = false;
    d->flags &= 0x00ff;
    d->flags |= quint16(flags) << 8;
}

bool Message::isUrgent() const
{
    return (d->flags & UrgentFlag) != 0;
}

bool Message::isReply() const
{
    return (d->flags & ReplyFlag) != 0;
}

quint32 Message::snr() const
{
    return d->snr;
}

void Message::setSnr(quint32 snr)
{
    d->isNull = false;
    d->snr = snr;
}

QByteArray Message::source() const
{
    return d->source;
}

void Message::setSource(const QByteArray &source)
{
    d->isNull = false;
    d->source = source;
    d->source.truncate(MessageDeviceNameSize);
}

QByteArray Message::destination() const
{
    return d->destination;
}

void Message::setDestination(const QByteArray &destination)
{
    d->isNull = false;
    d->destination = destination;
    d->destination.truncate(MessageDeviceNameSize);
}

QByteArray Message::data() const
{
    return d->data;
}

void Message::setData(const QByteArray &data)
{
    d->isNull = false;
    d->data = data;
}

QByteArray Message::toRawMsg() const
{
    QByteArray msg = d->data.rightJustified(
                MessageHeaderSize + d->data.size(), '\0', false);

    char *p = msg.data();
    quint16 *pFlags = reinterpret_cast<quint16 *>(p + MessageFlagsPos);
    quint32 *pSnr = reinterpret_cast<quint32 *>(p + MessageSnrPos);
    quint32 *pDataLen = reinterpret_cast<quint32 *>(p + MessageDataLenPos);

    *pFlags = qToBigEndian(d->flags);
    *pSnr = qToBigEndian(d->snr);
    qCopy(d->source.constBegin(), d->source.constEnd(), p + MessageSourcePos);
    qCopy(d->destination.constBegin(), d->destination.constEnd(),
          p + MessageDestinationPos);
    *pDataLen = qToBigEndian(static_cast<quint32>(d->data.size()));
    qCopy(d->data.constBegin(), d->data.constEnd(), p + MessageHeaderSize);

    Q_ASSERT(msg.size() == MessageHeaderSize + d->data.size());
    return msg;
}

Message Message::fromRawMsg(const QByteArray &rawMsg)
{
    // the message must at least contain the header
    if (rawMsg.size() < MessageHeaderSize)
        return Message();

    const char *p = rawMsg.constData();

    quint32 dataSize = qFromBigEndian(
                *reinterpret_cast<const quint32 *>(p + MessageDataLenPos));

    // check if message size and data size are consistent
    if (rawMsg.size() != int(MessageHeaderSize + dataSize))
        return Message();

    quint16 flags = qFromBigEndian(
                *reinterpret_cast<const quint16 *>(p + MessageFlagsPos));
    quint32 snr = qFromBigEndian(
                *reinterpret_cast<const quint32 *>(p + MessageSnrPos));

    QByteArray source = rawMsg.mid(MessageSourcePos, MessageDeviceNameSize);
    QByteArray destination = rawMsg.mid(
                MessageDestinationPos, MessageDeviceNameSize);
    QByteArray data = rawMsg.right(dataSize);

    // update members
    return Message(flags, snr, source, destination, data);
}

Message Message::ackMessage(int errorCode) const
{
    return Message(
        d->flags | ReplyFlag | UrgentFlag,
        d->snr, d->destination, d->source,
        QByteArray::number(errorCode) + " ACK");
}

Message Message::replyMessage(const QByteArray &data, int errorCode) const
{
    return Message(
        d->flags | ReplyFlag,
        d->snr, d->destination, d->source,
        QByteArray::number(errorCode) + " " + (data.isEmpty() ? "FIN" : data));
}

} // namespace Dcp
