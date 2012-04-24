%Module dcpclient

%Import QtCore/QtCoremod.sip
%Import QtNetwork/QtNetworkmod.sip

%ModuleCode
#include <dcpclient/dcpclient.h>
using namespace Dcp;
%End

%Include message.sip
%Include messageparser.sip
%Include client.sip
%Include version.sip
