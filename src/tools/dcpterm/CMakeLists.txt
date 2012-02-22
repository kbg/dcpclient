project(dcpterm)

include_directories(
    ${QT_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(dcpterm_SRCS dcpterm.cpp dcptermwin.cpp configdialog.cpp cmdlineoptions.cpp)
qt4_automoc(${dcpterm_SRCS})
qt4_wrap_cpp(dcpterm_MOC_SRCS dcptermwin.h configdialog.h)
qt4_wrap_ui(dcpterm_UI_HDRS dcptermwin.ui configdialog.ui)

add_executable(dcpterm ${dcpterm_SRCS} ${dcpterm_MOC_SRCS} ${dcpterm_UI_HDRS})
target_link_libraries(dcpterm DcpClient
    ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS dcpterm RUNTIME DESTINATION bin)
