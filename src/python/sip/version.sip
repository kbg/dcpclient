namespace Dcp {

const char * versionString() /PyName=libraryVersion/;

const char * moduleVersion();
%MethodCode
    sipRes = DCPCLIENT_VERSION_STRING;
%End

}; // namespace Dcp
