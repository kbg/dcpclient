namespace Dcp {

class MessageParser
/NoDefaultCtors/
{
%TypeHeaderCode
#include <dcpclient/messageparser.h>
%End

public:
    MessageParser();
    virtual ~MessageParser();
    virtual void clear();
    virtual bool parse(const Dcp::Message &msg);
    QList<QByteArray> arguments() const;
    QByteArray joinedArguments() const;
    bool hasArguments() const;
    int numArguments() const;
};



class ReplyParser : public Dcp::MessageParser
/NoDefaultCtors/
{
%TypeHeaderCode
#include <dcpclient/messageparser.h>
%End

public:
    ReplyParser();
    void clear();
    bool parse(const Dcp::Message &msg);
    bool isAckReply() const;
    int errorCode() const;
};


class CommandParser : public Dcp::MessageParser
/NoDefaultCtors/
{
%TypeHeaderCode
#include <dcpclient/messageparser.h>
%End

public:
    enum CmdType {
        SetCmd,
        GetCmd,
        DefCmd,
        UndefCmd
    };

    CommandParser();
    void clear();
    bool parse(const Dcp::Message &msg);
    CmdType cmdType() const;
    QByteArray command() const;
    QByteArray identifier() const;
};

}; // namespace Dcp
