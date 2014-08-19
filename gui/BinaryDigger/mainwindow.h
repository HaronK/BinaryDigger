
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include "treemodel.h"
#include "../../include/bd.h"
#include "FileTemplBlob.h"

#include <QFile>
#include <QLibrary>
#include <QLabel>
#include <QMainWindow>
#include <QModelIndex>
#include <QToolBar>
#include <QTreeView>
#include <QListWidget>
#include <QTextEdit>

#include <qhexedit2/qhexedit.h>

#include <scripteditor.h>
#include <syntaxhighlighter.h>

#include <optionsdialog.h>
#include <searchdialog.h>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QUndoStack;
QT_END_NAMESPACE

class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    struct PluginLibrary
    {
        QString file;
        QLibrary *library;

        QString pluginName;
        bd_plugin plugin;
        bool isScripter;
        QStringList templates;
    };

    int pluginIndex;
    QVector<PluginLibrary> plugins;

    QFunctionPointer loadPluginFunction(QLibrary *library, const char *name);
    void loadPlugin(PluginLibrary &pl);
    void loadPlugins();

    void initializePlugin(PluginLibrary &pl);
    void applyTemplate(bd_block *block);
    void freeTemplate(bd_block *block);
    void finalizePlugin(PluginLibrary &pl);

    void resizeTreeColumns();

    FileTemplBlob templBlob;
    bd_u32 templIndex;
    bd_block *rootBlock;

    // QHexEdit
protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void treeItemExpanded();
    void treeItemSelected();
    void pluginTemplActivated();

private slots:
    void about();
    void optionsAccepted();
    void findNext();
    void open();
    bool save();
    bool saveAs();
    void saveSelectionToReadableFile();
    void saveToReadableFile();

    void openScript();
    bool saveScript();
    bool saveScriptAs();

    void setAddress(int address);
    void setOverwriteMode(bool mode);
    void setSize(int size);
    void showOptionsDialog();
    void showSearchDialog();

private:
    void init();
    void fillPluginsWidget();
    void createActions();
    void createMenus();
    void createStatusBar();
    void createToolBars();

    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);

    void loadScriptFile(const QString &fileName);
    bool saveScriptFile(const QString &fileName);
    void setCurrentScriptFile(const QString &fileName);

    void readSettings();
    QString strippedName(const QString &fullFileName);
    void writeSettings();

    QString curFile;
    bool isUntitled;

    QString curScriptFile;
    bool isScriptUntitled;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *helpMenu;

    QToolBar *fileToolBar;

    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *saveReadable;
    QAction *closeAct;
    QAction *exitAct;

    QAction *openScriptAct;
    QAction *saveScriptAct;
    QAction *saveScriptAsAct;

    QAction *undoAct;
    QAction *redoAct;
    QAction *saveSelectionReadable;

    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *optionsAct;
    QAction *findAct;
    QAction *findNextAct;

    QTreeView *treeView;
    QStringList treeModelHeaders;
    TreeModel *treeModel;
    QListWidget *templWidget;
    ScriptEditor *scriptEditor;
    SyntaxHighlighter *highlighter;
    QHexEdit *hexEdit;

    OptionsDialog *optionsDialog;
    SearchDialog *searchDialog;

    QLabel *lbAddress, *lbAddressName;
    QLabel *lbOverwriteMode, *lbOverwriteModeName;
    QLabel *lbSize, *lbSizeName;

};

#endif // MAINWINDOW_H
