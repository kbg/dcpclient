if(BUILD_EXAMPLES)
    add_subdirectory(dcpdump)
    add_subdirectory(dcplisten)
    add_subdirectory(dcptime)
endif()

if(INSTALL_EXAMPLE_SOURCES)
    install(FILES dcpdump/dcpdump.h
                  dcpdump/dcpdump.cpp
                  dcpdump/dcpdump_main.cpp
            DESTINATION share/dcpclient/examples/dcpdump)
    install(FILES dcplisten/dcplisten.cpp
            DESTINATION share/dcpclient/examples/dcplisten)
    install(FILES dcptime/dcptimeserver.h
                  dcptime/dcptimeserver.cpp
                  dcptime/dcptimeserver_main.cpp
                  dcptime/dcptimeclient.h
                  dcptime/dcptimeclient.cpp
                  dcptime/dcptimeclient_main.cpp
            DESTINATION share/dcpclient/examples/dcptime)
endif()
