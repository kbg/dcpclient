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

#ifndef DCPCLIENT_VERSION_H
#define DCPCLIENT_VERSION_H

#define DCPCLIENT_VERSION_MAJOR @DCPCLIENT_VERSION_MAJOR@
#define DCPCLIENT_VERSION_MINOR @DCPCLIENT_VERSION_MINOR@
#define DCPCLIENT_VERSION_RELEASE @DCPCLIENT_VERSION_RELEASE@
#define DCPCLIENT_VERSION_STRING "@DCPCLIENT_VERSION_STRING@"

#define DCPCLIENT_VERSION \
    ((DCPCLIENT_VERSION_MAJOR << 16) | \
     (DCPCLIENT_VERSION_MINOR << 8) | \
     (DCPCLIENT_VERSION_RELEASE))

namespace Dcp {
    unsigned int versionMajor();
    unsigned int versionMinor();
    unsigned int versionRelease();
    const char * versionString();
}

#endif // DCPCLIENT_VERSION_H
