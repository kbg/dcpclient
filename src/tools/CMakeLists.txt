if(BUILD_TOOLS)
    add_subdirectory(dcpsend)
    if(QT_QTGUI_FOUND)
        add_subdirectory(dcpterm)
    else()
        message("Skipping build of dcpterm because of missing QtGui library.")
    endif()
endif()

if(INSTALL_EXAMPLE_SOURCES)
    install(FILES dcpsend/dcpsend.cpp
            DESTINATION share/dcpclient/examples/dcpsend)
    install(FILES dcpterm/dcpterm.cpp
                  dcpterm/dcptermwin.h
                  dcpterm/dcptermwin.cpp
                  dcpterm/dcptermwin.ui
            DESTINATION share/dcpclient/examples/dcpterm)
endif()
