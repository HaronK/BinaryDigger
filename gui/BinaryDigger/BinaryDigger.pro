QT += widgets
CONFIG += c++11

FORMS       = \
    mainwindow.ui \
    searchdialog.ui \
    optionsdialog.ui

#HEXEDIT_HDR = qhexedit2/*.h
HEXEDIT_HDR = ../../external/QHexEdit_Dax89/*.h

HEADERS     = \
    $$HEXEDIT_HDR \
    mainwindow.h \
    treeitem.h \
    treemodel.h \
    ../../include/*.h \
    BDException.h \
    searchdialog.h \
    optionsdialog.h \
    FileTemplBlob.h \
    syntaxhighlighter.h \
    scripteditor.h \
    hexedit/hexedit1.h \
    hexedit/hexedit2.h \
    hexedit/hexedit.h

#HEXEDIT_SRC = qhexedit2/*.cpp
HEXEDIT_SRC = ../../external/QHexEdit_Dax89/*.cpp

SOURCES     = \
    $$HEXEDIT_SRC \
    mainwindow.cpp \
    treeitem.cpp \
    treemodel.cpp \
    main.cpp \
    searchdialog.cpp \
    optionsdialog.cpp \
    FileTemplBlob.cpp \
    syntaxhighlighter.cpp \
    scripteditor.cpp

RESOURCES   = BinaryDigger.qrc

INCLUDEPATH += $$PWD/../../external

OTHER_FILES += \
    qhexedit2/license.txt \
    syntax/lua.json
