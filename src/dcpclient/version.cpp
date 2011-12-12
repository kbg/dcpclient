#include "version.h"

namespace Dcp {

unsigned int versionMajor()
{
    return DCPCLIENT_VERSION_MAJOR;
}

unsigned int versionMinor()
{
    return DCPCLIENT_VERSION_MINOR;
}

unsigned int versionRelease()
{
    return DCPCLIENT_VERSION_RELEASE;
}

const char * versionString()
{
    return DCPCLIENT_VERSION_STRING;
}

} // namespace Dcp
