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

class DcpMessageParserPrivate
{
public:
    DcpMessageParserPrivate();
    virtual ~DcpMessageParserPrivate();

    DcpMessageParser *q_ptr;
    QList<QByteArray> args;
    Q_DECLARE_PUBLIC(DcpMessageParser)
};

DcpMessageParserPrivate::DcpMessageParserPrivate()
    : q_ptr(0)
{
}

DcpMessageParserPrivate::~DcpMessageParserPrivate()
{
}


DcpMessageParser::DcpMessageParser()
    : d_ptr(new DcpMessageParserPrivate)
{
    d_ptr->q_ptr = this;
}

DcpMessageParser::DcpMessageParser(DcpMessageParserPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
}

DcpMessageParser::~DcpMessageParser()
{
    delete d_ptr;
}

bool DcpMessageParser::parse(const DcpMessage &msg, bool strict)
{
    Q_D(DcpMessageParser);
    QByteArray data = strict ? msg.data() : msg.data().simplified();
    d->args = data.split(' ');
    return true;
}

QList<QByteArray> DcpMessageParser::arguments()
{
    return QList<QByteArray>();
}

} // namespace Dcp
