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

#include "dcppacket.h"
#include <dcpclient/message.h>

void DcpPacket::setData(const QByteArray &data)
{
    m_data = data;
    const int size = data.size();

    // check overal packet size
    if (size < FullHeaderSize || size > MaxPacketSize) {
        m_data.clear();
        return;
    }

    // the message size and offset can be ignored, but the size of the
    // message data needs to be checked against the packet size
    quint32 msgDataSize = qFromBigEndian(*reinterpret_cast<const quint32 *>(
        m_data.constData() + PacketHeaderSize + MessageDataLenPos));
    if (msgDataSize + FullHeaderSize != size) {
        m_data.clear();
        return;
    }
}

Dcp::Message DcpPacket::message() const
{
    return Dcp::Message::fromByteArray(m_data.mid(PacketHeaderSize));
}
