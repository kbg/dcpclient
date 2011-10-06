#ifndef DCPMESSAGE_H
#define DCPMESSAGE_H

#include <QtCore/QByteArray>

class DcpMessage
{
public:
    enum {
        PaceFlag   = 0x01,
        GrecoFlag  = 0x02,
        UrgentFlag = 0x04,
        ReplyFlag  = 0x08
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
    bool hasPaceFlag() const;
    bool hasGrecoFlag() const;
    bool hasUrgentFlag() const;
    bool hasReplyFlag() const;

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

private:
    void init(const QByteArray &rawMsg);
    void init(quint16 flags, quint32 snr, const QByteArray &source,
              const QByteArray &destination, const QByteArray &data);

private:
    bool m_null;
    quint16 m_flags;
    quint32 m_snr;
    QByteArray m_source;
    QByteArray m_destination;
    QByteArray m_data;
};

#endif // DCPMESSAGE_H
