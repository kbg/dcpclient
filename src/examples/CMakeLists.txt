if(BUILD_EXAMPLES)
    add_subdirectory(dcpdump)
    add_subdirectory(dcplisten)
    add_subdirectory(dcptime)
endif()

if(INSTALL_EXAMPLE_SOURCES)
    configure_file(dcpdump/dcpdump.pro.in
                   ${CMAKE_CURRENT_BINARY_DIR}/dcpdump.pro)
    configure_file(dcplisten/dcplisten.pro.in
                   ${CMAKE_CURRENT_BINARY_DIR}/dcplisten.pro)
    configure_file(dcptime/dcptimeserver.pro.in
                   ${CMAKE_CURRENT_BINARY_DIR}/dcptimeserver.pro)
    configure_file(dcptime/dcptimeclient.pro.in
                   ${CMAKE_CURRENT_BINARY_DIR}/dcptimeclient.pro)
    install(FILES dcpdump/dcpdump.h
                  dcpdump/dcpdump.cpp
                  dcpdump/dcpdump_main.cpp
                  ${CMAKE_CURRENT_BINARY_DIR}/dcpdump.pro
            DESTINATION share/doc/dcpclient/examples/dcpdump)
    install(FILES dcplisten/dcplisten.cpp
                  ${CMAKE_CURRENT_BINARY_DIR}/dcplisten.pro
            DESTINATION share/doc/dcpclient/examples/dcplisten)
    install(FILES dcptime/dcptimeserver.h
                  dcptime/dcptimeserver.cpp
                  dcptime/dcptimeserver_main.cpp
                  ${CMAKE_CURRENT_BINARY_DIR}/dcptimeserver.pro
            DESTINATION share/doc/dcpclient/examples/dcptime/server)
    install(FILES dcptime/dcptimeclient.h
                  dcptime/dcptimeclient.cpp
                  dcptime/dcptimeclient_main.cpp
                  ${CMAKE_CURRENT_BINARY_DIR}/dcptimeclient.pro
            DESTINATION share/doc/dcpclient/examples/dcptime/client)
endif()
