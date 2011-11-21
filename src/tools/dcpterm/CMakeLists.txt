project(dcpterm)

include_directories(
    ${QT_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${libDcpClient_SOURCE_DIR}
)

set(dcpterm_SRCS
    dcpterm.cpp
    dcptermwin.cpp
)
qt4_automoc(${dcpterm_SRCS})
qt4_wrap_ui(dcpterm_UI_HDRS dcptermwin.ui)

add_executable(dcpterm ${dcpterm_SRCS} ${dcpterm_UI_HDRS})
target_link_libraries(dcpterm DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY})