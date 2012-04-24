class MessageParser
/NoDefaultCtors/
{
%TypeHeaderCode
#include <dcpclient/messageparser.h>
using namespace Dcp;
%End

public:
    MessageParser();
    virtual ~MessageParser();
    virtual void clear();
    virtual bool parse(const Message &msg);
    QList<QByteArray> arguments() const;
    QByteArray joinedArguments() const;
    bool hasArguments() const;
    int numArguments() const;
};



class ReplyParser : public MessageParser
/NoDefaultCtors/
{
%TypeHeaderCode
#include <dcpclient/messageparser.h>
using namespace Dcp;
%End

public:
    ReplyParser();
    void clear();
    bool parse(const Message &msg);
    bool isAckReply() const;
    int errorCode() const;
};


class CommandParser : public MessageParser
/NoDefaultCtors/
{
%TypeHeaderCode
#include <dcpclient/messageparser.h>
using namespace Dcp;
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
    bool parse(const Message &msg);
    CmdType cmdType() const;
    QByteArray command() const;
    QByteArray identifier() const;
};
