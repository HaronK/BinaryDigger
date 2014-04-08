QT += widgets

FORMS       = \
    mainwindow.ui \
    searchdialog.ui \
    optionsdialog.ui

HEADERS     = \
    mainwindow.h \
    treeitem.h \
    treemodel.h \
    ../../include/*.h \
    BDException.h \
    searchdialog.h \
    optionsdialog.h \
    qhexedit2/xbytearray.h \
    qhexedit2/qhexedit.h \
    qhexedit2/qhexedit_p.h \
    qhexedit2/commands.h \
    FileTemplBlob.h \
    syntaxhighlighter.h \
    scripteditor.h

SOURCES     = \
    mainwindow.cpp \
    treeitem.cpp \
    treemodel.cpp \
    main.cpp \
    qhexedit2/commands.cpp \
    qhexedit2/qhexedit.cpp \
    qhexedit2/qhexedit_p.cpp \
    qhexedit2/xbytearray.cpp \
    searchdialog.cpp \
    optionsdialog.cpp \
    FileTemplBlob.cpp \
    syntaxhighlighter.cpp \
    scripteditor.cpp

RESOURCES   = BinaryDigger.qrc

OTHER_FILES += \
    qhexedit2/license.txt \
    syntax/lua.json
