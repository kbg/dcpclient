/*
 * Copyright (c) 2012 Kolja Glogowski
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

#ifndef DCPHUB_DCPPACKET_H
#define DCPHUB_DCPPACKET_H

#include <QtCore/QtEndian>
#include <QtCore/QByteArray>

// DCP packet/message structure (copied from dcpclient_p.h)
enum {
    MessageHeaderSize = 42,
    MessageDeviceNameSize = 16,
    MessageFlagsPos = 0,
    MessageSnrPos = 2,
    MessageSourcePos = 6,
    MessageDestinationPos = 22,
    MessageDataLenPos = 38,
    PacketHeaderSize = 8,
    PacketMsgSizePos = 0,
    PacketOffsetPos = 4,
    FullHeaderSize = MessageHeaderSize + PacketHeaderSize,
    MaxPacketSize = 0x10000
};

namespace Dcp {
    class Message;
}

class DcpPacket
{
public:
    enum {
        PaceFlag   = 0x01,
        GrecoFlag  = 0x02,
        UrgentFlag = 0x04,
        ReplyFlag  = 0x08
    };

    DcpPacket() {}
    explicit DcpPacket(const QByteArray &data) { setData(data); }

    QByteArray data() const { return m_data; }
    void setData(const QByteArray &data);

    void clear() { m_data.clear(); }
    quint32 size() const { return m_data.size(); }
    bool isValid() const { return !m_data.isEmpty(); }

    quint16 flags() const;
    QByteArray source() const;
    QByteArray destination() const;

    Dcp::Message message() const;

private:
    QByteArray m_data;
};

inline quint16 DcpPacket::flags() const {
    return qFromBigEndian(*reinterpret_cast<const quint16 *>(
        m_data.constData() + PacketHeaderSize + MessageFlagsPos));
}

inline QByteArray DcpPacket::source() const {
    return m_data.mid(
        PacketHeaderSize + MessageSourcePos, MessageDeviceNameSize);
}

inline QByteArray DcpPacket::destination() const {
    return m_data.mid(
        PacketHeaderSize + MessageDestinationPos, MessageDeviceNameSize);
}

#endif // DCPHUB_DCPPACKET_H
