/*
 * Copyright (c) 2012 Kolja Glogowski
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

#ifndef HEXFORMATTER_H
#define HEXFORMATTER_H

#include <QByteArray>

class HexFormatter
{
public:
    enum DisplayOption {
        ShowNone     = 0x00,
        ShowPosition = 0x01,
        ShowText     = 0x02
    };

    explicit HexFormatter(int bytesPerLine = 0, int dispOpts = ShowNone,
                          char nonVisChar = '.');

    int bytesPerLine() const;
    void setBytesPerLine(int bytesPerLine);

    int displayOptions() const;
    void setDisplayOptions(int dispOpts);

    bool showPosEnabled() const;
    void setShowPosEnabled(bool enable);

    bool showTextEnabled() const;
    void setShowTextEnabled(bool enable);

    char nonVisibleChar() const;
    void setNonVisibleChar(char nonVisChar);

    const char * toHex(const char *ba, int size);
    QByteArray toHex(const QByteArray &ba) const;
    QByteArray operator()(const QByteArray &ba) const;
    void toHex(const QByteArray &ba, QByteArray *out) const;
    void toHex(const char *ba, int size, QByteArray *out) const;

private:
    QByteArray m_buf;
    int m_bytesPerLine;
    int m_dispOpts;
    char m_nonVisChar;
};

#endif // HEXFORMATTER_H
