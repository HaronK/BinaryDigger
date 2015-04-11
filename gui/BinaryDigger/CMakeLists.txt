
set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_ui(UI_HEADERS
    mainwindow.ui
    searchdialog.ui
    optionsdialog.ui
)

qt5_add_resources(UI_RESOURCES
    BinaryDigger.qrc
)

#set(QT_HEXEDIT_SRC
#    qhexedit2/qhexedit.cpp
#    qhexedit2/qhexedit_p.cpp
#    qhexedit2/commands.cpp
#    qhexedit2/xbytearray.cpp
#)

include_directories("../../external")
file(GLOB QT_HEXEDIT_SRC "../../external/QHexEdit_Dax89/*.cpp")

set(QT_BD_SRC
    ${QT_HEXEDIT_SRC}
    mainwindow.cpp
    treemodel.cpp
    treeitem.cpp
    main.cpp
    FileTemplBlob.cpp
    searchdialog.cpp
    optionsdialog.cpp
    scripteditor.cpp
    syntaxhighlighter.cpp
)

add_executable(BinaryDigger ${QT_BD_SRC} ${UI_HEADERS} ${UI_RESOURCES})

target_link_libraries(BinaryDigger ${Qt5Core_QTMAIN_LIBRARIES})

add_custom_command(TARGET BinaryDigger POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       $<TARGET_FILE:BinaryDigger>
                       "${BD_PACKAGE_FOLDER}"
                   COMMENT "BinaryDigger copied to ${BD_PACKAGE_FOLDER}"
                  )

qt5_use_modules(BinaryDigger Core Gui Widgets)
