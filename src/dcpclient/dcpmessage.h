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

#ifndef DCPMESSAGE_H
#define DCPMESSAGE_H

#include <QtCore/QByteArray>

namespace Dcp {

enum AckErrorCode {
    AckNoError = 0,
    AckUnknownCommandError = 2,
    AckParameterError = 3,
    AckWrongModeError = 5
};

class DcpMessage
{
public:
    enum {
        PaceFlag   = 0x1,
        GrecoFlag  = 0x2,
        UrgentFlag = 0x4,
        ReplyFlag  = 0x8,
        AckFlags   = UrgentFlag | ReplyFlag
    };

    DcpMessage();
    DcpMessage(const DcpMessage &other);
    explicit DcpMessage(const QByteArray &rawMsg);
    DcpMessage(quint16 flags, quint32 snr, const QByteArray &source,
               const QByteArray &destination, const QByteArray &data);

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

    QByteArray toRawMsg() const;
    static DcpMessage fromRawMsg(const QByteArray &rawMsg);

    DcpMessage ackMessage(int errorCode = AckNoError) const;
    DcpMessage replyMessage(const QByteArray &data, int errorCode = 0) const;

private:
    //! \todo Move to private d_ptr class.
    void init(const QByteArray &rawMsg);
    void init(quint16 flags, quint32 snr, const QByteArray &source,
              const QByteArray &destination, const QByteArray &data);

private:
    //! \todo Use implicit sharing, see QSharedDataPointer.
    bool m_null;
    quint16 m_flags;
    quint32 m_snr;
    QByteArray m_source;
    QByteArray m_destination;
    QByteArray m_data;
};

} // namespace Dcp

#endif // DCPMESSAGE_H
