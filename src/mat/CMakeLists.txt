add_executable(qtxdg-mat
    matcommandmanager.cpp
    matcommandinterface.cpp
    defappmatcommand.cpp
    openmatcommand.cpp
    mimetypematcommand.cpp
    defwebbrowsermatcommand.cpp
    defemailclientmatcommand.cpp
    deffilemanagermatcommand.cpp
    defterminalmatcommand.cpp

    qtxdg-mat.cpp
)

target_compile_definitions(qtxdg-mat
    PRIVATE
        "-DQTXDG_TOOLS_VERSION=\"${QTXDG_TOOLS_VERSION_STRING}\""
        "QT_NO_KEYWORDS"
)

target_link_libraries(qtxdg-mat
    Qt6::Core
    Qt6Xdg
)

install(TARGETS
    qtxdg-mat
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
    EXPORT "qtxdg-tools-targets"
    COMPONENT Runtime
)
