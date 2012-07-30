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

#include "hexformatter.h"

HexFormatter::HexFormatter(int bytesPerLine, int dispOpts, char nonVisChar)
    : m_bytesPerLine(bytesPerLine > 0 ? bytesPerLine : 0),
      m_nonVisChar(nonVisChar)
{
    setDisplayOptions(dispOpts);
}

int HexFormatter::bytesPerLine() const {
    return m_bytesPerLine;
}

void HexFormatter::setBytesPerLine(int bytesPerLine) {
    m_bytesPerLine = (bytesPerLine > 0 ? bytesPerLine : 0);
}

int HexFormatter::displayOptions() const {
    return m_dispOpts;
}

void HexFormatter::setDisplayOptions(int dispOpts) {
    m_dispOpts = ShowNone;
    m_dispOpts |= dispOpts & ShowPosition;
    m_dispOpts |= dispOpts & ShowText;
}

bool HexFormatter::showPosEnabled() const {
    return m_dispOpts & ShowPosition;
}

void HexFormatter::setShowPosEnabled(bool enable) {
    m_dispOpts = enable ? (m_dispOpts | ShowPosition)
                        : (m_dispOpts & ~ShowPosition);
}

bool HexFormatter::showTextEnabled() const {
    return m_dispOpts & ShowText;
}

void HexFormatter::setShowTextEnabled(bool enable) {
    m_dispOpts = enable ? (m_dispOpts | ShowText)
                        : (m_dispOpts & ~ShowText);
}

char HexFormatter::nonVisibleChar() const {
    return m_nonVisChar;
}

void HexFormatter::setNonVisibleChar(char nonVisChar) {
    m_nonVisChar = nonVisChar;
}

const char * HexFormatter::toHex(const char *ba, int size) {
    toHex(ba, size, &m_buf);
    return m_buf.constData();
}

QByteArray HexFormatter::toHex(const QByteArray &ba) const {
    QByteArray out;
    toHex(ba.constData(), ba.size(), &out);
    return out;
}

QByteArray HexFormatter::operator()(const QByteArray &ba) const {
    QByteArray out;
    toHex(ba.constData(), ba.size(), &out);
    return out;
}

void HexFormatter::toHex(const QByteArray &ba, QByteArray *out) const {
    toHex(ba.constData(), ba.size(), out);
}

void HexFormatter::toHex(const char *ba, int size, QByteArray *out) const
{
    if (out == 0)
        return;

    if (ba == 0 || size <= 0) {
        out->clear();
        return;
    }

    const int bytesPerLine = (m_bytesPerLine <= 0 || m_bytesPerLine > size)
            ? size : m_bytesPerLine;
    const bool showPos = showPosEnabled();
    const bool showText = showTextEnabled();

    // number of lines and line breaks (\n chars)
    int numLineBreaks = (size - 1) / bytesPerLine;
    int numLines = numLineBreaks + 1;

    int numBytesLastLine = size - bytesPerLine * numLineBreaks;
    int padBytesLastLine = bytesPerLine - numBytesLastLine;

    // each input byte needs 3 chars for hexadecimal representation and a
    // trainling space, except for the last one in each line which
    // is not followed by a space char
    int outSize = 3 * size - numLines;

    //  each line contains a linebreak, except the last one
    outSize += numLineBreaks;

    // if positions are shown, we need 8+2 additional chars per line
    if (showPos)
        outSize += 10 * numLines;

    // if text is shown, we need bytesPerLine+2 additional chars per line
    // and the last line needs to be padded if it does not contain a full line
    if (showText) {
        outSize += (bytesPerLine + 2) * numLineBreaks + (numBytesLastLine + 2);
        if (padBytesLastLine != 0)
            outSize += 3 * padBytesLastLine;
    }

    out->resize(outSize);
    const uchar *pin = reinterpret_cast<const uchar *>(ba);
    uchar *pout = reinterpret_cast<uchar *>(out->data());

    for (int line = 0; line < numLines; ++line)
    {
        // true if we are at the last line
        const bool lastLine = (line + 1 >= numLines);

        // number of bytes to be written in the current line
        const int numBytes = lastLine ? numBytesLastLine : bytesPerLine;

        if (showPos) {
            // show number of the first byte in the line as a 8 character
            // long hexadecimal number followed by 2 spaces
            QByteArray posText = QByteArray::number(line * bytesPerLine, 16).
                    rightJustified(8, '0', true);
            for (int i = 0; i < 8; ++i)
                *(pout++) = posText[i];
            *(pout++) = ' ';
            *(pout++) = ' ';
        }

        // write a hexadecimal representation of the data, with each byte
        // separated by a space character
        for (int i = 0; i < numBytes; ++i) {
            int v = (pin[i] >> 4) & 0xf;
            *(pout++) = (v < 10) ? ('0' + v) : ('a' + v - 10);
            v = pin[i] & 0xf;
            *(pout++) = (v < 10) ? ('0' + v) : ('a' + v - 10);
            if ((i + 1) < numBytes)
                *(pout++) = ' ';
        }

        if (showText) {
            // space padding: 2 + additional padding for the last line
            int n = 2;
            if (lastLine && padBytesLastLine != 0)
                n += 3 * padBytesLastLine;
            while (n-- > 0)
                *(pout++) = ' ';

            // write a textual representation of the data, with nonvisiual
            // characters replaced by m_nonvisChar
            for (int i = 0; i < numBytes; ++i) {
                uchar v = pin[i];
                *(pout++) = (v >= 0x20 && v <= 0x7e) ? v : m_nonVisChar;
            }
        }

        // write linebreak for all lines except the last one
        if (!lastLine)
            *(pout++) = '\n';

        pin += numBytes;
    }

    Q_ASSERT(pin == reinterpret_cast<const uchar *>(ba + size));
    Q_ASSERT(pout == reinterpret_cast<uchar *>(out->data() + outSize));
}
