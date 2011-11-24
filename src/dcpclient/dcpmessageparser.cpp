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

void MessageParser::clear()
{
    Q_D(MessageParser);
    d->args.clear();
}

bool MessageParser::parse(const Message &msg)
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

class ReplyParserPrivate : public MessageParserPrivate
{
public:
    ReplyParserPrivate();
    ReplyParser::ReplyType replyType;
    int errorCode;
};

ReplyParserPrivate::ReplyParserPrivate()
    : replyType(ReplyParser::EoeReply),
      errorCode(0)
{
}

ReplyParser::ReplyParser()
    : MessageParser(*(new ReplyParserPrivate))
{
}

ReplyParser::ReplyParser(ReplyParserPrivate &dd)
    : MessageParser(dd)
{
}

void ReplyParser::clear()
{
    Q_D(ReplyParser);
    MessageParser::clear();
    d->replyType = EoeReply;
    d->errorCode = 0;
}

bool ReplyParser::parse(const Message &msg)
{
    Q_D(ReplyParser);
    clear();

    // make sure that this is a reply message
    if (!msg.isReply())
        return false;

    if (!MessageParser::parse(msg))
        return false;

    if (d->args.isEmpty())
        return false;

    bool ok;
    d->errorCode = d->args.takeFirst().toInt(&ok);
    if (!ok)
        return false;

    if ((d->args.size() == 1) && (d->args[0] == "ACK"))
        d->replyType = AckReply;

    return true;
}

ReplyParser::ReplyType ReplyParser::replyType() const
{
    Q_D(const ReplyParser);
    return d->replyType;
}

bool ReplyParser::isAckReply() const
{
    Q_D(const ReplyParser);
    return d->replyType == AckReply;
}

bool ReplyParser::isEoeReply() const
{
    Q_D(const ReplyParser);
    return d->replyType == EoeReply;
}

int ReplyParser::errorCode() const
{
    Q_D(const ReplyParser);
    return d->errorCode;
}

// --------------------------------------------------------------------------

class CommandParserPrivate : public MessageParserPrivate
{
public:
    CommandParserPrivate();
    QByteArray command;
    QByteArray identifier;
    CommandParser::CommandType commandType;
};

CommandParserPrivate::CommandParserPrivate()
    : commandType(CommandParser::SetCommand)
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

void CommandParser::clear()
{
    Q_D(CommandParser);
    MessageParser::clear();
    d->command = QByteArray();
    d->identifier = QByteArray();
    d->commandType = SetCommand;
}

bool CommandParser::parse(const Message &msg)
{
    Q_D(CommandParser);
    clear();

    // make sure that this is not a reply message
    if (msg.isReply())
        return false;

    if (!MessageParser::parse(msg))
        return false;

    // command messages need at least a command keyword and an identifier
    if (d->args.size() < 2)
        return false;

    d->command = d->args.takeFirst();
    d->identifier = d->args.takeFirst();

    // parse command type
    if (d->command == "set")
        d->commandType = SetCommand;
    else if (d->command == "get")
        d->commandType = GetCommand;
    else if (d->command == "def")
        d->commandType = DefCommand;
    else if (d->command == "undef")
        d->commandType = UndefCommand;
    else
        return false;

    return true;
}

CommandParser::CommandType CommandParser::commandType() const
{
    Q_D(const CommandParser);
    return d->commandType;
}

QByteArray CommandParser::command() const
{
    Q_D(const CommandParser);
    return d->command;
}

QByteArray CommandParser::identifier() const
{
    Q_D(const CommandParser);
    return d->identifier;
}

} // namespace Dcp
