#include "mainwindow.h"

#include "treeitem.h"

#include <BDException.h>

#include <QFileDialog>
#include <QSplitter>

#include <QDir>
#include <QMessageBox>
#include <QException>

#define BD_CHECK(plugin, func, msg) \
    bd_result res = plugin.func; \
    if (!BD_SUCCEED(res)) { \
        char buf[1024]; \
        plugin.result_message(res, (bd_string) buf, sizeof(buf)); \
        throw BDException(tr("%1\n%2").arg(msg).arg(buf)); \
    }


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi(this);

    pluginIndex = -1;
    templIndex = 0;
    rootBlock = 0;
    treeModelHeaders << tr("Name") << tr("Value") << tr("Type") << tr("Start") << tr("Size");

    try
    {
        loadPlugins();
        init();
        setCurrentFile("");
        setCurrentScriptFile("");
    }
    catch (const BDException &ex)
    {
        QMessageBox::warning(this, tr("Plugin: "), tr(ex.what()), QMessageBox::Ok);
    }
}

MainWindow::~MainWindow()
{
    try
    {
        freeTemplate(&rootBlock);
    }
    catch (const BDException &ex)
    {
        QMessageBox::warning(this, tr("Plugin: "), tr(ex.what()), QMessageBox::Ok);
    }

    for (int i = 0; i < plugins.size(); ++i)
    {
        PluginLibrary &pl = plugins[i];
        try
        {
            finalizePlugin(pl);

            pl.library->unload();
            delete pl.library;
            pl.library = 0;
        }
        catch (const BDException &ex)
        {
            QMessageBox::warning(this, tr("Plugin: ") + pl.file, tr(ex.what()), QMessageBox::Ok);
        }
    }
}

QFunctionPointer MainWindow::loadPluginFunction(QLibrary *library, const char *name)
{
    QFunctionPointer result = library->resolve(name);
    if (result == 0)
        throw BDException(tr("Cannot load plugin function '%1' from '%2' library").arg(name).arg(library->fileName()));
    return result;
}

void MainWindow::loadPlugin(PluginLibrary &pl)
{
    pl.library = new QLibrary(pl.file);

    if (!pl.library->load())
        throw BDException(tr("Cannot load plugin: %1").arg(pl.file));

    try
    {
        pl.plugin.result_message    = (bd_result_message_t)    loadPluginFunction(pl.library, "bd_result_message");
        pl.plugin.initialize_plugin = (bd_initialize_plugin_t) loadPluginFunction(pl.library, "bd_initialize_plugin");
        pl.plugin.finalize_plugin   = (bd_finalize_plugin_t)   loadPluginFunction(pl.library, "bd_finalize_plugin");
        pl.plugin.template_name     = (bd_template_name_t)     loadPluginFunction(pl.library, "bd_template_name");
        pl.plugin.apply_template    = (bd_apply_template_t)    loadPluginFunction(pl.library, "bd_apply_template");
        pl.plugin.free_template     = (bd_free_template_t)     loadPluginFunction(pl.library, "bd_free_template");
        pl.plugin.get_string_value  = (bd_get_string_value_t)  loadPluginFunction(pl.library, "bd_get_string_value");
    }
    catch (const BDException &/*ex*/)
    {
        pl.library->unload();
        delete pl.library;
        pl.library = 0;

        throw;
    }
}

void MainWindow::loadPlugins()
{
    QDir pluginsFolder(tr("plugins"));

    QStringList nameFilter;
    nameFilter << "*.so";
    QFileInfoList allFiles = pluginsFolder.entryInfoList(nameFilter, QDir::Files);

    plugins.resize(allFiles.size());

    if (allFiles.size() == 0)
        throw BDException(tr("There are no plugins in %1").arg(pluginsFolder.absolutePath()));

    int pluginIndex = 0;
    for (QFileInfoList::iterator iter = allFiles.begin(); iter != allFiles.end(); ++iter)
    {
        PluginLibrary &pl = plugins[pluginIndex];
        pl.file = tr("plugins/") + iter->fileName();

        try
        {
            loadPlugin(pl);
            initializePlugin(pl);
        }
        catch (const BDException &ex)
        {
            pl.library->unload();
            delete pl.library;
            pl.library = 0;

            QMessageBox::warning(this, tr("Plugin: %1").arg(pl.file), tr(ex.what()), QMessageBox::Ok);
            continue;
        }
        pluginIndex++;
    }
    if (pluginIndex == 0)
        throw BDException(tr("There are no loadable plugins"));

    plugins.resize(pluginIndex);
}

void MainWindow::initializePlugin(PluginLibrary &pl)
{
    bd_char name[256];
    bd_u32 templ_count;

    BD_CHECK(pl.plugin, initialize_plugin(name, sizeof(name), &templ_count), tr("Cannot initialize plugin"));

    pl.pluginName = name;
    pl.isScripter = (templ_count == 0);

    for (bd_u32 i = 0; i < templ_count; ++i)
    {
        BD_CHECK(pl.plugin, template_name(i, name, sizeof(name)), tr("Cannot retrieve template name: %1").arg(i));

        pl.templates.append(name);
    }
}

void MainWindow::applyTemplate(bd_block **block)
{
    if (pluginIndex == -1 || block == NULL)
        return;

    if (templBlob.dataFile.isNull() || !templBlob.dataFile->isOpen())
    {
        QMessageBox::warning(this, tr("Data file"), tr("Data file is not open."), QMessageBox::Ok);
        return;
    }

    freeTemplate(block);

    std::string script;

    if (plugins[pluginIndex].isScripter)
        script = curScriptFile.toStdString();

    templBlob.dataFile->seek(0);

    bd_result res = plugins[pluginIndex].plugin.apply_template(templIndex, &templBlob, block,
                                                               script.empty() ? 0 :(bd_cstring) script.c_str());
    if (res != BD_SUCCESS)
    {
        char buf[1024]; \
        plugins[pluginIndex].plugin.result_message(res, (bd_string) buf, sizeof(buf));

        QMessageBox::critical(this, tr("Script: ") + curScriptFile,
                              tr("%1\n%2").arg(tr("Could not apply template 0")).arg(buf), QMessageBox::Ok);
    }

    treeModel = new TreeModel(treeModelHeaders, &plugins[pluginIndex].plugin, &templBlob, *block);
    treeView->setModel(treeModel);
    initTreeViewChangeSelection();

    resizeTreeColumns();
}

void MainWindow::freeTemplate(bd_block **block)
{
    if (pluginIndex == -1 || block == NULL || *block == NULL)
        return;

    if (!BD_SUCCEED(plugins[pluginIndex].plugin.free_template(templIndex, *block)))
    {
        // TODO: duplication looks not very nice. Use BOOST_SCOPE_EXIT or equivalent
        treeView->setModel(0);
        pluginIndex = -1;
        templIndex = 0;
        *block = NULL;

        throw BDException(tr("Could not free template 0"));
    }

    treeView->setModel(0);
    pluginIndex = -1;
    templIndex = 0;
    *block = NULL;
}

void MainWindow::finalizePlugin(PluginLibrary &pl)
{
    BD_CHECK(pl.plugin, finalize_plugin(), tr("Could not finalize plugin."));
}

void MainWindow::treeItemExpanded()
{
    resizeTreeColumns();
}

void MainWindow::treeItemSelected(const QModelIndex &selected, const QModelIndex &deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    QModelIndex curIndex = treeView->selectionModel()->currentIndex();
    if (!curIndex.isValid())
        return;

    const TreeModel::UserData& curItem = treeModel->getBlock(curIndex)->getUserData();
    if (curItem.block != 0)
    {
        bd_u64 begin = curItem.block->offset + (curItem.index == (bd_u32)-1 ? 0 : curItem.index * curItem.block->elem_size);
        bd_u64 size = curItem.index == (bd_u32)-1 ? curItem.block->size : curItem.block->elem_size;
        hexEdit->setSelectionRange(begin, begin + size);
    }
}

void MainWindow::pluginTemplActivated()
{
    QListWidgetItem *curItem = templWidget->currentItem();
    if (curItem == 0)
        return;

    pluginIndex = curItem->data(Qt::UserRole + 1).toInt();
    templIndex = curItem->data(Qt::UserRole + 2).toInt();

    try
    {
        applyTemplate(&rootBlock);
    }
    catch (const BDException& ex)
    {
        QMessageBox::warning(this, tr("Template: ") + curItem->text(), tr(ex.what()), QMessageBox::Ok);
    }
}

void MainWindow::resizeTreeColumns()
{
    for (int column = 0; column < treeView->model()->columnCount(); ++column)
        treeView->resizeColumnToContents(column);
}

// QHexEdit

/*****************************************************************************/
/* Protected methods */
/*****************************************************************************/
void MainWindow::closeEvent(QCloseEvent *)
{
    writeSettings();
}

/*****************************************************************************/
/* Private Slots */
/*****************************************************************************/
void MainWindow::about()
{
   QMessageBox::about(this, tr("About Binary Digger"),
            tr(""));
}

void MainWindow::optionsAccepted()
{
    writeSettings();
    readSettings();
}

void MainWindow::findNext()
{
    searchDialog->findNext();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open data file"), dataFolder);
    if (!fileName.isEmpty())
    {
        QDir dir;
        dataFolder = dir.absoluteFilePath(fileName);

        loadFile(fileName);
        setWindowTitle(tr("%1 - Binary Digger").arg(fileName));

        freeTemplate(&rootBlock);
    }
    else
    {
        setWindowTitle(tr("Binary Digger"));
    }
}

bool MainWindow::save()
{
    if (isUntitled)
        return saveAs();
    return saveFile(dataFile);
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), dataFile);
    if (fileName.isEmpty())
        return false;

    QDir dir;
    dataFolder = dir.absoluteFilePath(fileName);

    return saveFile(fileName);
}

void MainWindow::saveSelectionToReadableFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save To Readable File"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox::warning(this, tr("QHexEdit"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return;
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
        file.write(hexEdit->selectionToReadableString().toLatin1());
        QApplication::restoreOverrideCursor();

        statusBar()->showMessage(tr("File saved"), 2000);
    }
}

void MainWindow::saveToReadableFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save To Readable File"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Text))
        {
            QMessageBox::warning(this, tr("BinaryDigger"),
                                 tr("Cannot write file %1:\n%2.")
                                 .arg(fileName)
                                 .arg(file.errorString()));
            return;
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);
        file.write(hexEdit->toReadableString().toLatin1());
        QApplication::restoreOverrideCursor();

        statusBar()->showMessage(tr("File saved"), 2000);
    }
}

void MainWindow::openScript()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open script"), scriptFolder);
    if (!fileName.isEmpty())
    {
        QDir dir;
        scriptFolder = dir.absoluteFilePath(fileName);

        loadScriptFile(fileName);

//        freeTemplate(&rootItem);
    }
}

bool MainWindow::saveScript()
{
    if (isScriptUntitled)
        return saveScriptAs();
    return saveScriptFile(curScriptFile);
}

bool MainWindow::saveScriptAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save script As"), curScriptFile);
    if (fileName.isEmpty())
        return false;

    QDir dir;
    scriptFolder = dir.absoluteFilePath(fileName);

    return saveScriptFile(fileName);
}

void MainWindow::setAddress(int address)
{
    lbAddress->setText(QString("%1").arg(address, 1, 16));
}

void MainWindow::setOverwriteMode(bool mode)
{
    if (mode)
        lbOverwriteMode->setText(tr("Overwrite"));
    else
        lbOverwriteMode->setText(tr("Insert"));
}

void MainWindow::setSize(int size)
{
    lbSize->setText(QString("%1").arg(size));
}

void MainWindow::showOptionsDialog()
{
    optionsDialog->show();
}

void MainWindow::showSearchDialog()
{
    searchDialog->show();
}

/*****************************************************************************/
/* Private Methods */
/*****************************************************************************/
void MainWindow::init()
{
    QApplication::setStyle("plastique");

    // create widgets
    hexEdit  = new QHexEdit;
    hexEdit->setMinimumWidth(300);
    hexEdit->setMinimumHeight(300);

    templWidget = new QListWidget();
    templWidget->setSortingEnabled(true);
    fillPluginsWidget();

    treeView = new QTreeView;

    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    scriptEditor = new ScriptEditor();
    scriptEditor->setFont(font);

    // Prepare highlighter
    // TODO: setting highlighter should heppen every time after script loading
    highlighter = new SyntaxHighlighter(scriptEditor->document());
    highlighter->initSyntax(":/syntax/lua.json");

    // setup layouts
    QWidget *myCentralWidget = new QWidget;

    QSplitter *leftSplitter = new QSplitter(this);
    leftSplitter->setOrientation(Qt::Vertical);
    leftSplitter->addWidget(hexEdit);
    leftSplitter->addWidget(templWidget);
    leftSplitter->setStretchFactor(0, 1);

    QSplitter *rightSplitter = new QSplitter(this);
    rightSplitter->setOrientation(Qt::Vertical);
    rightSplitter->addWidget(treeView);
    rightSplitter->addWidget(scriptEditor);
//    leftSplitter->setStretchFactor(0, 1);

    QSplitter *mainSplitter = new QSplitter(this);
    mainSplitter->addWidget(leftSplitter);
    mainSplitter->addWidget(rightSplitter);
//    mainSplitter->setStretchFactor(0, 0);
//    mainSplitter->setStretchFactor(1, 1);

    QHBoxLayout *mainLayout = new QHBoxLayout(myCentralWidget);
    mainLayout->addWidget(mainSplitter);

    myCentralWidget->setLayout(mainLayout);

    setCentralWidget(myCentralWidget);

    optionsDialog = new OptionsDialog(this);
    connect(optionsDialog, SIGNAL(accepted()), this, SLOT(optionsAccepted()));
    isUntitled = true;

    connect(hexEdit, SIGNAL(overwriteModeChanged(bool)), this, SLOT(setOverwriteMode(bool)));
    searchDialog = new SearchDialog(hexEdit, this);

    connect(treeView, SIGNAL(expanded(QModelIndex)), this, SLOT(treeItemExpanded()));

    connect(templWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(pluginTemplActivated()));

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    readSettings();

    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::initTreeViewChangeSelection()
{
    connect(treeView->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
            this, SLOT(treeItemSelected(const QModelIndex &, const QModelIndex &)));
}

void MainWindow::fillPluginsWidget()
{
    for (int i = 0; i < plugins.size(); ++i)
    {
        PluginLibrary &pl = plugins[i];

        if (pl.isScripter)
        {
            QListWidgetItem *item = new QListWidgetItem(tr("%1").arg(pl.pluginName));
            item->setData(Qt::UserRole + 1, QVariant(i));
            item->setData(Qt::UserRole + 2, QVariant(0));
            templWidget->addItem(item);
            continue;
        }

        for (int j = 0; j < pl.templates.size(); ++j)
        {
            QListWidgetItem *item = new QListWidgetItem(tr("%1::%2").arg(pl.pluginName).arg(pl.templates[j]));
            item->setData(Qt::UserRole + 1, QVariant(i));
            item->setData(Qt::UserRole + 2, QVariant(j));
            templWidget->addItem(item);
        }
    }
}

void MainWindow::createActions()
{
    // File
    openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    saveReadable = new QAction(tr("Save &Readable..."), this);
    saveReadable->setStatusTip(tr("Save document in readable form"));
    connect(saveReadable, SIGNAL(triggered()), this, SLOT(saveToReadableFile()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    // Script
    openScriptAct = new QAction(QIcon(":/images/open.png"), tr("Open script..."), this);
//    openScriptAct->setShortcuts(QKeySequence::Open);
    openScriptAct->setStatusTip(tr("Open an existing script file"));
    connect(openScriptAct, SIGNAL(triggered()), this, SLOT(openScript()));

    saveScriptAct = new QAction(QIcon(":/images/save.png"), tr("Save script"), this);
//    saveScriptAct->setShortcuts(QKeySequence::Save);
    saveScriptAct->setStatusTip(tr("Save the script to disk"));
    connect(saveScriptAct, SIGNAL(triggered()), this, SLOT(saveScript()));

    saveScriptAsAct = new QAction(tr("Save script As..."), this);
//    saveScriptAsAct->setShortcuts(QKeySequence::SaveAs);
    saveScriptAsAct->setStatusTip(tr("Save the script under a new name"));
    connect(saveScriptAsAct, SIGNAL(triggered()), this, SLOT(saveScriptAs()));

    // Edit
    undoAct = new QAction(tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    connect(undoAct, SIGNAL(triggered()), hexEdit, SLOT(undo()));

    redoAct = new QAction(tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    connect(redoAct, SIGNAL(triggered()), hexEdit, SLOT(redo()));

    saveSelectionReadable = new QAction(tr("&Save Selection Readable..."), this);
    saveSelectionReadable->setStatusTip(tr("Save selection in readable form"));
    connect(saveSelectionReadable, SIGNAL(triggered()), this, SLOT(saveSelectionToReadableFile()));

    findAct = new QAction(tr("&Find/Replace"), this);
    findAct->setShortcuts(QKeySequence::Find);
    findAct->setStatusTip(tr("Show the Dialog for finding and replacing"));
    connect(findAct, SIGNAL(triggered()), this, SLOT(showSearchDialog()));

    findNextAct = new QAction(tr("Find &next"), this);
    findNextAct->setShortcuts(QKeySequence::FindNext);
    findNextAct->setStatusTip(tr("Find next occurrence of the searched pattern"));
    connect(findNextAct, SIGNAL(triggered()), this, SLOT(findNext()));

    // Help
    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    optionsAct = new QAction(tr("&Options"), this);
    optionsAct->setStatusTip(tr("Show the Dialog to select applications options"));
    connect(optionsAct, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(saveReadable);
    fileMenu->addSeparator();
    fileMenu->addAction(openScriptAct);
    fileMenu->addAction(saveScriptAct);
    fileMenu->addAction(saveScriptAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addAction(saveSelectionReadable);
    editMenu->addSeparator();
    editMenu->addAction(findAct);
    editMenu->addAction(findNextAct);
    editMenu->addSeparator();
    editMenu->addAction(optionsAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
}

void MainWindow::createStatusBar()
{
    // Address Label
    lbAddressName = new QLabel();
    lbAddressName->setText(tr("Address:"));
    statusBar()->addPermanentWidget(lbAddressName);
    lbAddress = new QLabel();
    lbAddress->setFrameShape(QFrame::Panel);
    lbAddress->setFrameShadow(QFrame::Sunken);
    lbAddress->setMinimumWidth(70);
    statusBar()->addPermanentWidget(lbAddress);
    connect(hexEdit, SIGNAL(currentAddressChanged(int)), this, SLOT(setAddress(int)));

    // Size Label
    lbSizeName = new QLabel();
    lbSizeName->setText(tr("Size:"));
    statusBar()->addPermanentWidget(lbSizeName);
    lbSize = new QLabel();
    lbSize->setFrameShape(QFrame::Panel);
    lbSize->setFrameShadow(QFrame::Sunken);
    lbSize->setMinimumWidth(70);
    statusBar()->addPermanentWidget(lbSize);
    connect(hexEdit, SIGNAL(currentSizeChanged(int)), this, SLOT(setSize(int)));

    // Overwrite Mode Label
    lbOverwriteModeName = new QLabel();
    lbOverwriteModeName->setText(tr("Mode:"));
    statusBar()->addPermanentWidget(lbOverwriteModeName);
    lbOverwriteMode = new QLabel();
    lbOverwriteMode->setFrameShape(QFrame::Panel);
    lbOverwriteMode->setFrameShadow(QFrame::Sunken);
    lbOverwriteMode->setMinimumWidth(70);
    statusBar()->addPermanentWidget(lbOverwriteMode);
    setOverwriteMode(hexEdit->overwriteMode());

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName(tr("FileToolBar"));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);
    fileToolBar->addSeparator();
    fileToolBar->addAction(openScriptAct);
    fileToolBar->addAction(saveScriptAct);
}

void MainWindow::loadFile(const QString &fileName)
{
    templBlob.dataFile.reset(new QFile(fileName));

    if (!templBlob.dataFile->open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, tr("SDI"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(templBlob.dataFile->errorString()));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    hexEdit->setFile(templBlob.dataFile);
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("BinaryDigger"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    hexEdit->saveTo(file);
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::loadScriptFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("BinaryDigger"),
                             tr("Cannot read script %1:\n%2")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    scriptEditor->setPlainText(file.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentScriptFile(fileName);
    statusBar()->showMessage(tr("Script loaded"), 2000);
}

bool MainWindow::saveScriptFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("BinaryDigger"),
                             tr("Cannot write script %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    file.write(scriptEditor->toPlainText().toLatin1()); // FIXME: should it be really Latin1?
    QApplication::restoreOverrideCursor();

    setCurrentScriptFile(fileName);
    statusBar()->showMessage(tr("Script saved"), 2000);
    return true;
}

void MainWindow::readSettings()
{
    QSettings settings("Haron", "BinaryDigger");

    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    dataFolder = settings.value("dataFolder").toString();
    scriptFolder = settings.value("scriptFolder").toString();

//    hexEdit->setAddressArea(settings.value("AddressArea").toBool());
//    hexEdit->setAsciiArea(settings.value("AsciiArea").toBool());
//    hexEdit->setHighlighting(settings.value("Highlighting").toBool());
//    hexEdit->setOverwriteMode(settings.value("OverwriteMode").toBool());
//    hexEdit->setReadOnly(settings.value("ReadOnly").toBool());

//    hexEdit->setHighlightingColor(settings.value("HighlightingColor").value<QColor>());
//    hexEdit->setAddressAreaColor(settings.value("AddressAreaColor").value<QColor>());
//    hexEdit->setSelectionColor(settings.value("SelectionColor").value<QColor>());
//    hexEdit->setFont(settings.value("WidgetFont").value<QFont>());

//    hexEdit->setAddressWidth(settings.value("AddressAreaWidth").toInt());
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    dataFile = QFileInfo(fileName).canonicalFilePath();
    isUntitled = fileName.isEmpty();
    setWindowModified(false);
    setWindowFilePath(dataFile);
}

void MainWindow::setCurrentScriptFile(const QString &fileName)
{
    curScriptFile = QFileInfo(fileName).canonicalFilePath();
    isScriptUntitled = fileName.isEmpty();
    setWindowModified(false);
//    setWindowFilePath(curScriptFile);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::writeSettings()
{
    QSettings settings("Haron", "BinaryDigger");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("geometry",    saveGeometry());
    settings.setValue("windowState", saveState());
}
