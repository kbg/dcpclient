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

#include "message.h"
#include "dcpclient_p.h"
#include <QtCore/QSharedData>
#include <QtCore/QtEndian>
#include <QtCore/QtAlgorithms>
#include <QtCore/QString>
#include <QtCore/QDebug>

namespace Dcp {

/*! \class Message
    \brief DCP message class.
 */

/*! \brief Returns a string describing the given AckErrorCode. */
QString ackErrorString(int errorCode)
{
    switch (errorCode)
    {
    case Dcp::AckNoError:
        return QString("No Error");
    case Dcp::AckUnknownCommandError:
        return QString("Unknown Command");
    case Dcp::AckParameterError:
        return QString("Parameter Error");
    case Dcp::AckWrongModeError:
        return QString("Wrong Mode");
    default:
        return QString("Unknown Error");
    }
}

/*! \brief Returns a copy of \a input with percent-encoded space characters.

    This function replaces all space characters (<code>' '</code>) in \a input
    by the string <code>"%20"</code> and any percent characters
    (<code>'\%'</code>) by <code>"%25"</code>.

    \note This is a simplified version of the QByteArray::toPercentEncoding()
          method; you can use the QByteArray::fromPercentEncoding() method
          to decode the resulting string.
 */
QByteArray percentEncodeSpaces(const QByteArray &input)
{
    return QByteArray(input).replace("%", "%25").replace(" ", "%20");
}

/*! \internal
    \brief Implicitly shared message data.
    \todo Use a static null-object instead of the isNull flag. This needs to
          be carefully implemented to keep the Message class reentrant.
 */
class MessageData : public QSharedData
{
public:
    MessageData() : isNull(true), flags(0), snr(0) {}
    MessageData(const MessageData &other) : QSharedData(other),
            isNull(other.isNull), flags(other.flags), snr(other.snr),
            source(other.source), destination(other.destination),
            data(other.data) {}
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

/*! \brief Creates a null-message, i.e. isNull() returns true.

    \sa isNull()
 */
Message::Message()
    : d(new MessageData)
{
}

/*! \brief Copy constructor. */
Message::Message(const Message &other)
    : d(other.d)
{
}

/*! \brief Creates a Dcp::Message object.

    \param snr serial number of the message
    \param source name of the source device
    \param destination name of the destination device
    \param data the message data
    \param flags message flags (combined DCP and user flags)

    \sa snr(), source(), destination(), data(), flags()
 */
Message::Message(quint32 snr, const QByteArray &source,
                 const QByteArray &destination, const QByteArray &data,
                 quint16 flags = 0)
    : d(new MessageData(flags, snr, source, destination, data))
{
}

/*! \brief Creates a Dcp::Message object.

    \param snr serial number of the message
    \param source name of the source device
    \param destination name of the destination device
    \param data the message data
    \param dcpFlags DCP flags
    \param userFlags user flags

    \sa snr(), source(), destination(), data(), dcpFlags(), userFlags
 */
Message::Message(quint32 snr, const QByteArray &source,
                 const QByteArray &destination, const QByteArray &data,
                 quint8 dcpFlags, quint8 userFlags)
    : d(new MessageData(quint16(dcpFlags) | (quint16(userFlags) << 8),
                        snr, source, destination, data))
{
}

/*! \brief Destroys the message object. */
Message::~Message()
{
}

/*! \brief Assignement operator. */
Message & Message::operator=(const Message &other)
{
    d = other.d;
    return *this;
}

/*! \brief Clears the contents of the message.

    After calling this method the object is a null-object, i.e. isNull()
    returns true.

    \sa isNull()
 */
void Message::clear()
{
    d->isNull = true;
    d->flags = 0;
    d->snr = 0;
    d->source.clear();
    d->destination.clear();
    d->data.clear();
}

/*! \brief Returns true if the message is a null-message, otherwise returns
    false.
 */
bool Message::isNull() const
{
    return d->isNull;
}

/*! \brief Returns the message flags. */
quint16 Message::flags() const
{
    return d->flags;
}

/*! \brief Sets the message flags. */
void Message::setFlags(quint16 flags)
{
    d->isNull = false;
    d->flags = flags;
}

/*! \brief Returns the DCP part of the message flags. */
quint8 Message::dcpFlags() const
{
    return quint8(d->flags & 0x00ff);
}

/*! \brief Sets the DCP part of the message flags. */
void Message::setDcpFlags(quint8 flags)
{
    d->isNull = false;
    d->flags &= 0xff00;
    d->flags |= quint16(flags);
}

/*! \brief Returns the user part of the message flags. */
quint8 Message::userFlags() const
{
    return quint8(d->flags >> 8);
}

/*! \brief Sets the user part of the message flags. */
void Message::setUserFlags(quint8 flags)
{
    d->isNull = false;
    d->flags &= 0x00ff;
    d->flags |= quint16(flags) << 8;
}

/*! \brief Returns true if the UrgentFlag is set; otherwise returns false. */
bool Message::isUrgent() const
{
    return (d->flags & UrgentFlag) != 0;
}

/*! \brief Returns true if the ReplyFlag is set; otherwise returns false. */
bool Message::isReply() const
{
    return (d->flags & ReplyFlag) != 0;
}

/*! \brief Returns the serial number of the message. */
quint32 Message::snr() const
{
    return d->snr;
}

/*! \brief Sets the serial number of the message. */
void Message::setSnr(quint32 snr)
{
    d->isNull = false;
    d->snr = snr;
}

/*! \brief Returns the name of the source device. */
QByteArray Message::source() const
{
    return d->source;
}

/*! \brief Sets the name of the source device. */
void Message::setSource(const QByteArray &source)
{
    d->isNull = false;
    d->source = source;
    d->source.truncate(MessageDeviceNameSize);
}

/*! \brief Returns the name of the destination device. */
QByteArray Message::destination() const
{
    return d->destination;
}

/*! \brief Sets the name of the destination device. */
void Message::setDestination(const QByteArray &destination)
{
    d->isNull = false;
    d->destination = destination;
    d->destination.truncate(MessageDeviceNameSize);
}

/*! \brief Returns the message data. */
QByteArray Message::data() const
{
    return d->data;
}

/*! \brief Sets the message data. */
void Message::setData(const QByteArray &data)
{
    d->isNull = false;
    d->data = data;
}

/*! \brief Converts the Message object to a QByteArray.

    \sa fromByteArray()
 */
QByteArray Message::toByteArray() const
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

/*! \brief Converts a QByteArray to a new Message object.

    This method parses a QByteArray and returns a new Message object. If
    something went wrong during the parsing, a null-message object is
    returned.

    \sa toByteArray(), Message::isNull()
 */
Message Message::fromByteArray(const QByteArray &rawMsg)
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

    return Message(snr, source, destination, data, flags);
}

/*! \brief Creates an ACK reply message.

    Use this method to create an ACK reply message from a previously received
    command message. The \a source and \a destination entries of the original
    message will be swapped for the resulting message, while the \a snr will
    remain unchanged. For ACK messages the Message::ReplyFlag and
    Message::UrgentFlag will be set.

    \param errorCode ACK error code; this should be an error code defined
           in the enumeration AckErrorCode.

    \sa AckErrorCode, replyMessage()
 */
Message Message::ackMessage(int errorCode) const
{
    return Message(
        d->snr, d->destination, d->source,
        QByteArray::number(errorCode) + " ACK",
        d->flags | ReplyFlag | UrgentFlag);
}

/*! \brief Creates a reply message.

    Use this method to create a reply message from a previously received
    command message (make sure to send an ACK reply first). The \a source
    and \a destination entries of the original message will be swapped for
    the resulting message, while the \a snr will remain unchanged. For reply
    messages the Message::ReplyFlag will be set.

    \param data the data of the reply message. This should be <code>FIN</code>
           if there is no data to be sent back.
    \param errorCode the error code of the reply message. If there is no
           error, \a errorCode should be 0. In case of an error \a errorCode
           should be a positive number defined by the application and \a data
           does not need to contain any valid data. If there is a warning that
           needs to be signalized, \a data must contain a valid set of data
           and \a errorCode should contain an application-defined negative
           number.

    \sa ackMessage()
 */
Message Message::replyMessage(const QByteArray &data, int errorCode) const
{
    return Message(
        d->snr, d->destination, d->source,
        QByteArray::number(errorCode) + " " + (data.isEmpty() ? "FIN" : data),
        d->flags | ReplyFlag);
}

/*! \brief QTextStream output operator for Dcp::Message objects.

    This output operator can be used to print a human-readable representation
    of a message object to a QTextStream.

    <b>Example:</b>
    \code
    #include <QtCore/QTextStream>
    QTextStream cout(stdout, QIODevice::WriteOnly);
    ...
    Dcp::Message msg(1, "src", "dst", "dat", 0);
    cout << msg << endl;
    \endcode

    \sa operator<<(QDebug debug, const Dcp::Message &msg)
 */
QTextStream & operator << (QTextStream &os, const Dcp::Message &msg)
{
    return os
        << ((msg.flags() & Dcp::Message::PaceFlag) != 0 ? "p" : "-")
        << ((msg.flags() & Dcp::Message::GrecoFlag) != 0 ? "g" : "-")
        << (msg.isUrgent() ? "u" : "-")
        << (msg.isReply() ? "r" : "-")
        << hex << " [0x" << msg.flags() << dec << "] "
        << "#" << msg.snr() << " "
        << "\"" << QString::fromLatin1(msg.source()) << "\" -> "
        << "\"" << QString::fromLatin1(msg.destination()) << "\" "
        << "[" << msg.data().size() << "] "
        << "\"" << QString::fromLatin1(msg.data()) << "\"";
}

/*! \brief QDebug output operator for Dcp::Message objects.

    This output operator can be used to print a human-readable representation
    of a message object to the qDebug() stream.

    <b>Example:</b>
    \code
    #include <QtCore/QtDebug>
    ...
    Dcp::Message msg(1, "src", "dst", "dat", 0);
    qDebug() << msg;
    \endcode

    \sa operator<<(QTextStream &os, const Dcp::Message &msg)
 */
QDebug operator << (QDebug debug, const Dcp::Message &msg)
{
    debug.nospace()
        << ((msg.flags() & Dcp::Message::PaceFlag) != 0 ? "p" : "-")
        << ((msg.flags() & Dcp::Message::GrecoFlag) != 0 ? "g" : "-")
        << (msg.isUrgent() ? "u" : "-")
        << (msg.isReply() ? "r" : "-")
        << hex << " [0x" << msg.flags() << dec << "] "
        << "#" << msg.snr() << " "
        << QString::fromLatin1(msg.source()) << " -> "
        << QString::fromLatin1(msg.destination()) << " "
        << "[" << msg.data().size() << "] "
        << QString::fromLatin1(msg.data());
    return debug.space();
}

} // namespace Dcp
