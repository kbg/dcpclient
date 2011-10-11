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

#include "dcpclient_p.h"
#include <QtCore/QByteArray>

namespace Dcp {

/*
    Removes all trailing characters with value c from the given byte array.
*/
void stripRight(QByteArray &ba, char c)
{
    int i = ba.size() - 1;
    for (; i >= 0; --i)
        if (ba.at(i) != c)
            break;
    ba.truncate(i+1);
}

/*
    Returns the time left for a timeout value, i.e. the time difference
    between msecs and elapsed with a lower bound of 0, or -1 if msecs has
    a value of -1.
 */
int timeoutValue(int msecs, int elapsed)
{
    if (msecs == -1)
        return -1;
    int msecsLeft = msecs - elapsed;
    return msecsLeft < 0 ? 0 : msecsLeft;
}

} // namespace Dcp
