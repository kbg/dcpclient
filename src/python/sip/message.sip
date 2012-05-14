namespace Dcp {

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
    Message(const Dcp::Message &other);
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

    quint16 flags() const /PyName=_getFlags/;
    void setFlags(quint16 flags) /PyName=_setFlags/;

    quint8 dcpFlags() const /PyName=_getDcpFlags, PyInt/;
    void setDcpFlags(quint8 flags /PyInt/) /PyName=_setDcpFlags/;

    quint8 userFlags() const /PyName=_getUserFlags, PyInt/;
    void setUserFlags(quint8 flags /PyInt/) /PyName=_setUserFlags/;

    bool isUrgent() const;
    bool isReply() const;

    quint32 snr() const /PyName=_getSnr/;
    void setSnr(quint32 snr) /PyName=_setSnr/;

    QByteArray source() const /PyName=_getSource/;
    void setSource(const QByteArray &source) /PyName=_setSource/;

    QByteArray destination() const /PyName=_getDestination/;
    void setDestination(const QByteArray &destination) /PyName=_setDestination/;

    QByteArray data() const /PyName=_getData/;
    void setData(const QByteArray &data) /PyName=_setData/;

    QByteArray toByteArray() const;
    static Dcp::Message fromByteArray(const QByteArray &rawMsg);

    Dcp::Message ackMessage(int errorCode = Dcp::AckNoError) const;
    Dcp::Message replyMessage(const QByteArray &data = QByteArray(),
                         int errorCode = 0) const;

public:
    %Property(name=flags, get=_getFlags, set=_setFlags)
    %Property(name=dcpFlags, get=_getDcpFlags, set=_setDcpFlags)
    %Property(name=userFlags, get=_getUserFlags, set=_setUserFlags)
    %Property(name=snr, get=_getSnr, set=_setSnr)
    %Property(name=source, get=_getSource, set=_setSource)
    %Property(name=destination, get=_getDestination, set=_setDestination)
    %Property(name=data, get=_getData, set=_setData)

    SIP_PYOBJECT __repr__() const /DocType="str"/;
%MethodCode
    QByteArray ba("Message(");
    if (!sipCpp->isNull()) {
        ba += QByteArray::number(sipCpp->snr()) + ", '";
        ba += sipCpp->source() + "', '";
        ba += sipCpp->destination() + "', '";
        ba += sipCpp->data() + "', ";
        ba += "0x" + QByteArray::number(sipCpp->flags(), 16) + ")";
    }
    else
        ba += ")";

#if PY_MAJOR_VERSION >= 3
    sipRes = PyUnicode_FromString(ba.constData());
#else
    sipRes = PyString_FromString(ba.constData());
#endif
%End // __repr__()
};

//QTextStream & operator << (QTextStream &os, const Dcp::Message &msg);
//QDebug operator << (QDebug debug, const Dcp::Message &msg);

}; // namespace Dcp
