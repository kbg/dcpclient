from dcp.dcpclient import Dcp

Client = Dcp.Client
Message = Dcp.Message
MessageParser = Dcp.MessageParser
ReplyParser = Dcp.ReplyParser
CommandParser = Dcp.CommandParser
AckErrorCode = Dcp.AckErrorCode
AckNoError = Dcp.AckNoError
AckUnknownCommandError = Dcp.AckUnknownCommandError
AckParameterError = Dcp.AckParameterError
AckWrongModeError = Dcp.AckWrongModeError
ackErrorString = Dcp.ackErrorString
percentEncodeSpaces = Dcp.percentEncodeSpaces

__all__ = [ 'Client', 'Message', 'MessageParser', 'ReplyParser',
    'CommandParser', 'AckErrorCode', 'AckNoError', 'AckUnknownCommandError',
    'AckParameterError', 'AckWrongModeError', 'ackErrorString',
    'percentEncodeSpaces' ]

__version__ = Dcp.moduleVersion()
__library_version__ = Dcp.libraryVersion()

#del dcp.dcpclient
#del Dcp, dcp
