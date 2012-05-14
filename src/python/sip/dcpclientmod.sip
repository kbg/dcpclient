%Module dcpclient

%Import QtCore/QtCoremod.sip
%Import QtNetwork/QtNetworkmod.sip

namespace Dcp {
%TypeHeaderCode
#include <dcpclient/dcpclient.h>
%End
};

%Include message.sip
%Include messageparser.sip
%Include client.sip
%Include version.sip
