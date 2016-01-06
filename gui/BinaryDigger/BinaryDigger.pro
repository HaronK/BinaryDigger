QT += widgets
CONFIG += c++11

FORMS       = \
    mainwindow.ui \
    searchdialog.ui \
    optionsdialog.ui

#HEXEDIT = QHexEdit_Dax89
HEXEDIT = qhexedit

HEXEDIT_HDR = ../../external/$$HEXEDIT/*.h

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
    hexedit/*.h

HEXEDIT_SRC = ../../external/$$HEXEDIT/*.cpp

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

INCLUDEPATH += $$PWD/../../external/$$HEXEDIT

OTHER_FILES += \
    qhexedit2/license.txt \
    syntax/lua.json
