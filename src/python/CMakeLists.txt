set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(PythonInterp REQUIRED)
find_package(SIP REQUIRED)
find_package(PyQt4 REQUIRED)

if(${CMAKE_VERBOSE_MAKEFILE})
    set(_verbose_output "True")
else()
    set(_verbose_output "False")
endif()
configure_file(configure.py.in ${CMAKE_CURRENT_BINARY_DIR}/configure.py)

set(pydcp_SIP
    ${CMAKE_CURRENT_SOURCE_DIR}/sip/dcpclientmod.sip
    ${CMAKE_CURRENT_SOURCE_DIR}/sip/client.sip
    ${CMAKE_CURRENT_SOURCE_DIR}/sip/message.sip
    ${CMAKE_CURRENT_SOURCE_DIR}/sip/messageparser.sip
    ${CMAKE_CURRENT_SOURCE_DIR}/sip/version.sip
)

set(pydcp_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/sipdcpclientcmodule.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipdcpclientDcp.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipdcpclientDcpClient.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipdcpclientDcpMessage.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipdcpclientDcpMessageParser.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipdcpclientDcpReplyParser.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipdcpclientDcpCommandParser.cpp
)

add_custom_command(
    OUTPUT ${pydcp_SRCS}
    COMMAND "${PYTHON_EXECUTABLE}" ${CMAKE_CURRENT_BINARY_DIR}/configure.py
    COMMENT "Generating C++ code for Python bindings..."
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/configure.py ${pydcp_SIP})

set_property(DIRECTORY APPEND PROPERTY
    ADDITIONAL_MAKE_CLEAN_FILES
        ${pydcp_SRCS}
        ${CMAKE_CURRENT_BINARY_DIR}/sipAPIdcpclient.h
        ${CMAKE_CURRENT_BINARY_DIR}/dcpclientmod.sbf)

include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    ${SIP_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src/dcpclient
)

add_library(pydcp MODULE ${pydcp_SRCS})
target_link_libraries(pydcp
    DcpClient
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY})

set_target_properties(pydcp PROPERTIES
    OUTPUT_NAME dcpclient
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX ""
)

execute_process(COMMAND "${PYTHON_EXECUTABLE}" -c
        "from distutils import sysconfig; print(sysconfig.get_python_lib(plat_specific=1, prefix='${CMAKE_INSTALL_PREFIX}'))"
    OUTPUT_VARIABLE _pydcp_install_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT _pydcp_install_path)
    message(FATAL_ERROR
        "Cannot determine installation directory for Python bindings.")
else()
    set(_pydcp_install_path ${_pydcp_install_path}/dcp)
endif()

install(TARGETS pydcp DESTINATION ${_pydcp_install_path})
install(FILES dcp/__init__.py
              dcp/parser.py
              dcp/dcpshell.py
              dcp/shell.py
        DESTINATION ${_pydcp_install_path})

unset(_verbose_output)
unset(_pydcp_install_path)
