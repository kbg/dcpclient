enum AckErrorCode {
    AckNoError = 0,
    AckUnknownCommandError = 2,
    AckParameterError = 3,
    AckWrongModeError = 5
};

QString ackErrorString(int errorCode);
QByteArray percentEncodeSpaces(const QByteArray &input);

class Message
{
%TypeHeaderCode
#include <dcpclient/message.h>
using namespace Dcp;
%End

public:
    enum {
        PaceFlag   = 0x01,
        GrecoFlag  = 0x02,
        UrgentFlag = 0x04,
        ReplyFlag  = 0x08,
        //AckFlags   = UrgentFlag | ReplyFlag
        AckFlags   = 0x0c
    };

    Message();
    Message(const Message &other);
    Message(quint32 snr, const QByteArray &source,
            const QByteArray &destination, const QByteArray &data,
            quint16 flags);
    Message(quint32 snr, const QByteArray &source,
            const QByteArray &destination, const QByteArray &data,
            quint8 dcpFlags /PyInt/, quint8 userFlags /PyInt/);
    ~Message();

    //Message & operator=(const Message &other);

    void clear();
    bool isNull() const;

    quint16 flags() const;
    void setFlags(quint16 flags);

    quint8 dcpFlags() const /PyInt/;
    void setDcpFlags(quint8 flags /PyInt/);

    quint8 userFlags() const /PyInt/;
    void setUserFlags(quint8 flags /PyInt/);

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
};

//QTextStream & operator << (QTextStream &os, const Dcp::Message &msg);
//QDebug operator << (QDebug debug, const Dcp::Message &msg);
