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

#include "dcpmessageparser.h"
#include "dcpmessage.h"
#include <QtCore/QList>
#include <QtCore/QByteArray>

namespace Dcp {

class MessageParserPrivate
{
public:
    MessageParserPrivate();
    virtual ~MessageParserPrivate();

    MessageParser *q_ptr;
    QList<QByteArray> args;
    Q_DECLARE_PUBLIC(MessageParser)
};

MessageParserPrivate::MessageParserPrivate()
    : q_ptr(0)
{
}

MessageParserPrivate::~MessageParserPrivate()
{
}


MessageParser::MessageParser()
    : d_ptr(new MessageParserPrivate)
{
    d_ptr->q_ptr = this;
}

MessageParser::MessageParser(MessageParserPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

MessageParser::~MessageParser()
{
    delete d_ptr;
}

bool MessageParser::parse(const DcpMessage &msg)
{
    Q_D(MessageParser);
    d->args = msg.data().split(' ');
    d->args.removeAll("");
    return true;
}

QList<QByteArray> MessageParser::arguments() const
{
    Q_D(const MessageParser);
    return d->args;
}

// --------------------------------------------------------------------------

ReplyParser::ReplyParser()
{

}

ReplyParser::~ReplyParser()
{

}

bool ReplyParser::parse(const DcpMessage &msg)
{

}

int ReplyParser::errorCode() const
{

}

// --------------------------------------------------------------------------

class CommandParserPrivate : public MessageParserPrivate
{
public:
    CommandParserPrivate();
    QByteArray cmdString;
    CommandParser::CommandType cmdType;
};

CommandParserPrivate::CommandParserPrivate()
    : cmdType(CommandParser::UnknownCommand)
{
}

CommandParser::CommandParser()
    : MessageParser(*(new CommandParserPrivate))
{
}

CommandParser::CommandParser(CommandParserPrivate &dd)
    : MessageParser(dd)
{
}

bool CommandParser::parse(const DcpMessage &msg)
{
    Q_D(CommandParser);

    if (!MessageParser::parse(msg)) {
        d->cmdString = QByteArray();
        d->cmdType = CommandParser::UnknownCommand;
        return false;
    }

    d->cmdString = d->args.isEmpty() ? QByteArray() : d->args.takeFirst();
    if (d->cmdString == "set")
        d->cmdType = SetCommand;
    else if (d->cmdString == "get")
        d->cmdType = GetCommand;
    else if (d->cmdString == "def")
        d->cmdType = DefCommand;
    else if (d->cmdString == "undef")
        d->cmdType = UndefCommand;
    else
        d->cmdType = UnknownCommand;

    return true;
}

CommandParser::CommandType CommandParser::commandType() const
{
    Q_D(const CommandParser);
    return d->cmdType;
}

QByteArray CommandParser::commandString() const
{
    Q_D(const CommandParser);
    return d->cmdString;
}

} // namespace Dcp
