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

#ifndef DCPCLIENT_PRIVATE_H
#define DCPCLIENT_PRIVATE_H

/*
    WARNING

    This is a private header file containing implementation details.
    Don't use this file as its content may change in future.
 */

class QByteArray;

namespace Dcp {

enum {
    DcpMessageHeaderSize = 42,
    DcpMessageDeviceNameSize = 16,
    DcpMessageFlagsPos = 0,
    DcpMessageSnrPos = 2,
    DcpMessageSourcePos = 6,
    DcpMessageDestinationPos = 22,
    DcpMessageDataLenPos = 38,
    DcpPacketHeaderSize = 8,
    DcpPacketMsgSizePos = 0,
    DcpPacketOffsetPos = 4,
    DcpFullHeaderSize = DcpMessageHeaderSize + DcpPacketHeaderSize,
    DcpMaxPacketSize = 0x10000
};

void stripRight(QByteArray &ba, char c = '\0');
int timeoutValue(int msecs, int elapsed);

} // namespace Dcp

#endif // DCPCLIENT_PRIVATE_H
