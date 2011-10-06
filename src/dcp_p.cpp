#include "dcp_p.h"
#include <QtCore/QByteArray>

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
