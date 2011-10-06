#ifndef DCP_PRIVATE_H
#define DCP_PRIVATE_H

/*
    WARNING

    This is a private header file containing implementation details.
    Don't use this file as its content may change in future.
 */

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

class QByteArray;
void stripRight(QByteArray &ba, char c = '\0');
int timeoutValue(int msecs, int elapsed);

#endif // DCP_PRIVATE_H
