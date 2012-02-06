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

#include "messageparser.h"
#include "message.h"
#include <QtCore/QList>
#include <QtCore/QByteArray>

namespace Dcp {

/*! \class MessageParser
    \brief A generic DCP message parser.

    The MessageParser class is the base class of all specialized message
    parsers, provided by this library. It parses the data part of a message
    by simply splitting the data into parts separated by spaces. The result
    can be accessed by the arguments() method.

    Usually more specialized parsers like ReplyParser or CommandParser should
    be used, which handle error codes or command types automatically.

    \sa ReplyParser, CommandParser
 */

/*! \class ReplyParser
    \brief Message parser for reply messages.

    This specialized message parser class can be used to parse reply
    messages.
 */

/*! \class CommandParser
    \brief Message parser for command messages.

    This specialized message parser class can be used to parse command
    messages.
 */

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

/*! \brief Creates a message parser object. */
MessageParser::MessageParser()
    : d_ptr(new MessageParserPrivate)
{
    d_ptr->q_ptr = this;
}

/*! \internal \brief Private data constructor. */
MessageParser::MessageParser(MessageParserPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

/*! Destroys the message parser object. */
MessageParser::~MessageParser()
{
    delete d_ptr;
}

/*! \brief Clears the results from the last parse() call. */
void MessageParser::clear()
{
    Q_D(MessageParser);
    d->args.clear();
}

/*! \brief Parses a DCP message.

    Returns true if the message was successfully parsed; otherwise
    returns false.

    This implementation simply splits the message data into parts that are
    separated by space characters (i.e. ASCII code 32). The resulting list can
    be accessed by using the arguments() method.

    \sa arguments()
 */
bool MessageParser::parse(const Message &msg)
{
    Q_D(MessageParser);
    d->args = msg.data().split(' ');
    d->args.removeAll("");
    return true;
}

/*! \brief Returns a list of arguments that were parsed from the last message.

    The arguments that are contained in this list depend on the particular
    implementation of the parse() method.

    \sa parse(), clear()
 */
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
    bool isAck;
    int errorCode;
};

ReplyParserPrivate::ReplyParserPrivate()
    : isAck(false),
      errorCode(0)
{
}

/*! \brief Creates a parser object for reply messages. */
ReplyParser::ReplyParser()
    : MessageParser(*(new ReplyParserPrivate))
{
}

/*! \internal \brief Private data constructor. */
ReplyParser::ReplyParser(ReplyParserPrivate &dd)
    : MessageParser(dd)
{
}

void ReplyParser::clear()
{
    Q_D(ReplyParser);
    MessageParser::clear();
    d->isAck = false;
    d->errorCode = 0;
}

/*! \brief Parses a reply message.

    Returns true if the reply message was successfully parsed; otherwise
    returns false.

    This implementation can be used to parse reply messages as specified by
    the DCP protocol. Each reply message contains an error code and additional
    arguments. When this method succeeds, the error code has been successfully
    parsed and can be accessed by using the errorCode() method. All remaining
    arguments can be accessed by using the arguments() method.

    A special class of reply message are ACK messages, which contain an error
    code followed by the single argument <code>"ACK"</code>. You can use the
    isAckReply() method to check if the last parsed message was an ACK reply.

    \sa errorCode(), arguments(), isAckReply()
 */
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
        d->isAck = true;

    return true;
}

/*! \brief Returns true if the last parsed message was an ACK reply; otherwise
           returns false.
 */
bool ReplyParser::isAckReply() const
{
    Q_D(const ReplyParser);
    return d->isAck;
}

/*! \brief Returns the error code of the last parsed message. */
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

/*! \brief Creates a parser object for command messages. */
CommandParser::CommandParser()
    : MessageParser(*(new CommandParserPrivate))
{
}

/*! \internal \brief Private data constructor. */
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

/*! \brief Parses a command message.

    Returns true if the command message was successfully parsed; otherwise
    returns false.

    This implementation can be used to parse command messages as specified by
    the DCP protocol. Each command message contains a command
      (<code>"set"</code>, <code>"get"</code>,
       <code>"def"</code>, <code>"undef"</code>),
    an identifier and optional further arguments. The command type can be
    accessed by using the methods command() or commandType(), the identifier
    by using the method identifier() and the remaining arguments by using the
    method arguments().

    \sa command(), commandType(), identifier(), arguments()
 */
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

/*! \brief Returns the command type of the last parsed message as
           enumeration. */
CommandParser::CommandType CommandParser::commandType() const
{
    Q_D(const CommandParser);
    return d->commandType;
}

/*! \brief Returns the command type of the last parsed message as string. */
QByteArray CommandParser::command() const
{
    Q_D(const CommandParser);
    return d->command;
}

/*! \brief Returns the identifier of the last parsed message. */
QByteArray CommandParser::identifier() const
{
    Q_D(const CommandParser);
    return d->identifier;
}

} // namespace Dcp
