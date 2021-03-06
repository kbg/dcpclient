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

#ifndef DCPCLIENT_MESSAGEPARSER_H
#define DCPCLIENT_MESSAGEPARSER_H

#include "dcpclient_export.h"

class QByteArray;
template <typename T> class QList;

namespace Dcp {

class Message;

class MessageParserPrivate;
class DCPCLIENT_EXPORT MessageParser
{
public:
    MessageParser();
    virtual ~MessageParser();
    virtual void clear();
    virtual bool parse(const Message &msg);
    QList<QByteArray> arguments() const;
    QByteArray joinedArguments() const;
    bool hasArguments() const;
    int numArguments() const;

protected:
    MessageParserPrivate * const d_ptr;
    MessageParser(MessageParserPrivate &dd);

private:
    Q_DISABLE_COPY(MessageParser)
    Q_DECLARE_PRIVATE(MessageParser)
};

class ReplyParserPrivate;
class DCPCLIENT_EXPORT ReplyParser : public MessageParser
{
public:
    ReplyParser();
    void clear();
    bool parse(const Message &msg);
    bool isAckReply() const;
    int errorCode() const;

protected:
    ReplyParser(ReplyParserPrivate &dd);

private:
    Q_DISABLE_COPY(ReplyParser)
    Q_DECLARE_PRIVATE(ReplyParser)
};

class CommandParserPrivate;
class DCPCLIENT_EXPORT CommandParser : public MessageParser
{
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

protected:
    CommandParser(CommandParserPrivate &dd);

private:
    Q_DISABLE_COPY(CommandParser)
    Q_DECLARE_PRIVATE(CommandParser)
};

} // namespace Dcp

#endif // DCPCLIENT_MESSAGEPARSER_H
