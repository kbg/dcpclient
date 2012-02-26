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

#ifndef DCPCLIENT_MESSAGE_H
#define DCPCLIENT_MESSAGE_H

#include <QtCore/QByteArray>
#include <QtCore/QSharedDataPointer>

namespace Dcp {

enum AckErrorCode {
    AckNoError = 0,
    AckUnknownCommandError = 2,
    AckParameterError = 3,
    AckWrongModeError = 5
};

QString ackErrorString(int errorCode);

class MessageData;
class Message
{
public:
    enum {
        PaceFlag   = 0x01,
        GrecoFlag  = 0x02,
        UrgentFlag = 0x04,
        ReplyFlag  = 0x08,
        AckFlags   = UrgentFlag | ReplyFlag
    };

    Message();
    Message(const Message &other);
    Message(quint32 snr, const QByteArray &source,
            const QByteArray &destination, const QByteArray &data,
            quint16 flags);
    Message(quint32 snr, const QByteArray &source,
            const QByteArray &destination, const QByteArray &data,
            quint8 dcpFlags, quint8 userFlags);
    ~Message();

    Message & operator=(const Message &other);

    void clear();
    bool isNull() const;

    quint16 flags() const;
    void setFlags(quint16 flags);

    quint8 dcpFlags() const;
    void setDcpFlags(quint8 flags);

    quint8 userFlags() const;
    void setUserFlags(quint8 flags);

    bool isUrgent() const;
    bool isReply() const;

    quint32 snr() const;
    void setSnr(quint32 snr);

    QByteArray source() const;
    void setSource(const QByteArray &source);

    QByteArray destination() const;
    void setDestination(const QByteArray &destination);

    QByteArray data() const;
    void setData(const QByteArray &data);

    QByteArray toByteArray() const;
    static Message fromByteArray(const QByteArray &rawMsg);

    Message ackMessage(int errorCode = AckNoError) const;
    Message replyMessage(const QByteArray &data = QByteArray(),
                         int errorCode = 0) const;

private:
    QSharedDataPointer<MessageData> d;
};

} // namespace Dcp

#endif // DCPCLIENT_MESSAGE_H
