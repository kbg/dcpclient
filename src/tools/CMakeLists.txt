if(BUILD_TOOLS)
    add_subdirectory(dcpsend)
    add_subdirectory(dcphub)
    if(TARGET Qt4::QtGui)
        add_subdirectory(dcpterm)
    else()
        message("Skipping build of dcpterm because of missing QtGui library.")
    endif()
endif()

if(INSTALL_EXAMPLE_SOURCES)
    configure_file(dcpsend/dcpsend.pro.in
                   ${CMAKE_CURRENT_BINARY_DIR}/dcpsend.pro)
    configure_file(dcpterm/dcpterm.pro.in
                   ${CMAKE_CURRENT_BINARY_DIR}/dcpterm.pro)
    install(FILES dcpsend/dcpsend.cpp
                  ${CMAKE_CURRENT_BINARY_DIR}/dcpsend.pro
            DESTINATION share/doc/dcpclient/examples/dcpsend)
    install(FILES dcpterm/dcpterm.cpp
                  dcpterm/dcptermwin.h
                  dcpterm/dcptermwin.cpp
                  dcpterm/dcptermwin.ui
                  dcpterm/configdialog.h
                  dcpterm/configdialog.cpp
                  dcpterm/configdialog.ui
                  dcpterm/cmdlineoptions.h
                  dcpterm/cmdlineoptions.cpp
                  ${CMAKE_CURRENT_BINARY_DIR}/dcpterm.pro
            DESTINATION share/doc/dcpclient/examples/dcpterm)
endif()
