#include "mainwindow.h"
#include <QMetaMethod>
#include <QStandardPaths>
#include <QMessageBox>

/**
 * Constructor for the MainWindow class.
 *
 * @param parent the parent widget
 *
 * @return None
 *
 * @throws None
 */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setMinimumSize(800, 600);
    // Try to read window geometry from settings
    QSettings qSettings;
    qSettings.beginGroup("WindowGeometry");
    restoreGeometry(qSettings.value("geometry").toByteArray());
    qSettings.endGroup();
    
    completer.setCaseSensitivity(Qt::CaseInsensitive);
    completer.setCompletionMode(QCompleter::PopupCompletion);

    createMenu();
    createOtherWidgets();
    setText();

    leDestinationDir.setCompleter(&completer);
    connectSignals();
}

/**
 * Destructor for the MainWindow class.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
MainWindow::~MainWindow() {
    // Save window dimensions
    QSettings qSettings;
    qSettings.beginGroup("WindowGeometry");
    qSettings.setValue("geometry", this->saveGeometry());
    qSettings.endGroup();
    
    // disconnect all SIGNALS
    const QMetaObject* metaObject = this->metaObject();

    for (int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            QObject::disconnect(nullptr, method.methodSignature().toStdString().c_str(), this, nullptr);
        }
    }
}

/**
 * Creates the main menu for the MainWindow class.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 *
 *    
 *  Menu structure:
 *  File    - Open File         : openFile()
 *          - Open Directory    : openPath()
 *          - Open FTP Server   - Add Profile   : addProfile()
 *                              - Profile 1     : openNetwork(1)
 *                              ....
 *                              - Profile n     : openNetwork(n)
 *          - Rename File       : rename()
 *          - Delete File       : deleteFile()
 *          - Settings          : settings()
 *          - Quit              : cancel()
 *  Help    - About             : About()
 */
void MainWindow::createMenu() {

    
    addToolBar(Qt::TopToolBarArea, &toolBar);

    openFileAction.setShortcuts(QKeySequence::Open);
    openFileAction.setStatusTip(tr("Open a single PDF file from local disk."));
    openFileAction.setIcon(QIcon(":/images/file.png"));
    QObject::connect(&openFileAction, &QAction::triggered, this, &MainWindow::openFile); 
    fileMenu->addAction(&openFileAction);
    toolBar.addAction(&openFileAction);

    openPathAction.setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    openPathAction.setStatusTip(tr("Open a directory from local diskw."));
    openPathAction.setIcon(QIcon(":/images/directory.png"));
    QObject::connect(&openPathAction, &QAction::triggered, this, &MainWindow::openPath);
    fileMenu->addAction(&openPathAction);
    toolBar.addAction(&openPathAction);

    networkProfileMenu = fileMenu->addMenu(tr("Remote &profiles"));
    fileMenu->addSeparator();

   // Get all network profile entries and create a menu entry for each of them
     for (int i=0; i < settings.networkProfileCount(); i++) {
        createNetworkMenuEntry(settings.NetworkProfile(i));
    }

    toolBar.insertWidget(&renameAction, &cbDocumentProfiles);
    cbDocumentProfiles.setToolTip(tr("The currently used document profile for processing the pdf files."));
    createDocumentEntries();

    renameAction.setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    renameAction.setIcon(QIcon(":/images/rename.png"));
    renameAction.setStatusTip(tr("Rename file to filename in destination directory."));
    QObject::connect(&renameAction, &QAction::triggered, this, &MainWindow::renameToFinalName);
    fileMenu->addAction(&renameAction);

    deleteAction.setShortcut(QKeySequence::Delete);
    deleteAction.setIcon(QIcon(":/images/delete.png"));
    deleteAction.setStatusTip(tr("Delete file."));
    QObject::connect(&deleteAction, &QAction::triggered, this, &MainWindow::deleteFileSlot);
    fileMenu->addAction(&deleteAction);
    toolBar.addAction(&deleteAction);

    fileMenu->addSeparator();
    
    settingsAction.setShortcut(QKeySequence::Preferences);
    settingsAction.setStatusTip(tr("Settings."));
    QObject::connect(&settingsAction, &QAction::triggered, this, &MainWindow::settingsMenu);
    fileMenu->addAction(&settingsAction);

    fileMenu->addSeparator();
    
    cancelAction.setShortcut(QKeySequence::Quit);
    cancelAction.setStatusTip(tr("Quit."));
    QObject::connect(&cancelAction, &QAction::triggered, this, &MainWindow::cancel);
    fileMenu->addAction(&cancelAction);

    fileMenu->addSeparator();

    aboutAction.setShortcut(QKeySequence::HelpContents);
    aboutAction.setStatusTip(tr("About."));
    QObject::connect(&aboutAction, &QAction::triggered, this, &MainWindow::about);
    fileMenu->addAction(&aboutAction);
}

/**
 * Creates and sets up the layout for the other widgets in the main window.
 * 
 * The layout is structured as follows:
 * 
 * |c1           |c2         |c3                |c4              |
 * -----------------------------------------------------------------
 * r0: Menu
 * -----------------------------------------------------------------
 * r0:Toolbar
 * -----------------------------------------------------------------
 * r1:                         | lbDestinationDir
 * -----------------------------------------------------------------                            
 * r2:|leFilename  | pbRename  | leDestinationDir | pbDestinationDir
 * -----------------------------------------------------------------
 * r3:|        lsFiles         |               pdfView              |
 * ------------------------------------------------------------------
 * r4: pbProgress
 * ------------------------------------------------------------------
 *
 * @throws None
 */
void MainWindow::createOtherWidgets() {
    centralWidget.setLayout(&mainLayout);
    setCentralWidget(&centralWidget);

    lsFiles.setMaximumWidth(400);
    leFileName.setMaximumWidth(240);
    
    pbRename.setFixedWidth(120);
    pbRename.setDefault(true);

    pbDestinationDir.setFixedWidth(25);

    mainLayout.setContentsMargins(10,10,10,10);

    mainLayout.addWidget(&lbDestinationDir, 1, 3);
    mainLayout.addWidget(&leFileName, 2, 1);
    mainLayout.addWidget(&pbRename, 2, 2);
    mainLayout.addWidget(&leDestinationDir, 2, 3);
    mainLayout.addWidget(&pbDestinationDir, 2, 4);
    mainLayout.addWidget(&lsFiles, 3, 1, 1, 2);
    mainLayout.addWidget(&pdfView, 3, 3, 1, 2);
    pdfView.setPageMode(QPdfView::PageMode::MultiPage);
    mainLayout.addWidget(&pbProgress, 4, 1, 1, 4);

    mainLayout.setColumnStretch(1, 1);
    mainLayout.setColumnStretch(2, 1);
    mainLayout.setColumnStretch(3, 2);
    mainLayout.setColumnStretch(4, 4);
    
}

/**
 * Creates a network menu entry for the given network profile.
 *
 * @param netProfile pointer to the network profile object
 *
 * @return None
 *
 * @throws None
 */
void MainWindow::createNetworkMenuEntry(Settings::networkProfile *netProfile) {
    // if it is the default network profile, add it to the toolbar
    if (netProfile->isDefault) {
        // New separator before renameAction
        toolBar.insertSeparator(&renameAction);

        // Setup toolbar button for default network profile
        tbDefaultNetworkEntry.setIcon(QIcon(":/images/network.png"));
        tbDefaultNetworkEntry.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        tbDefaultNetworkEntry.setText(netProfile->name.c_str());
        tbDefaultNetworkEntry.setProperty("Url", QVariant::fromValue(netProfile->url));
        tbDefaultNetworkEntry.setProperty("IsRecursive", QVariant::fromValue(netProfile->isRecursive));
        tbDefaultNetworkEntry.setProperty("DocumentProfileIndex", QVariant::fromValue(netProfile->documentProfileIndex));
        toolBar.insertWidget(&renameAction, &tbDefaultNetworkEntry);
        tbDefaultNetworkEntry.setToolTip(tr("Scan a directory on a remote server for pdf files."));

        QObject::connect(&tbDefaultNetworkEntry, &QToolButton::clicked, this, &MainWindow::openNetwork); 
            
        // if there are more entries, create a drop down menu
        if (settings.networkProfileCount() > 1) {
            tbNetworkProfiles.setIcon(QIcon(":/images/down_arrow.svg"));
            tbNetworkProfiles.setFixedWidth(10);
            tbDefaultNetworkEntry.setFixedWidth(networkProfileMenu->width()+ 10);
            tbNetworkProfileAction = toolBar.insertWidget(&renameAction, &tbNetworkProfiles);
            connect(&tbNetworkProfiles, &QToolButton::clicked, [&]() {
                QPoint menuPos = tbDefaultNetworkEntry.mapToGlobal(QPoint(0, tbDefaultNetworkEntry.height()));
                networkProfileMenu->exec(menuPos);
            });
        }
        toolBar.insertSeparator(&renameAction);
    }
    // Add Profile to NetworkProfileMenu
    QAction *networkProfileAction = new QAction(netProfile->name.c_str(), this);
    networkProfileAction->setIcon(QIcon(":/images/network.png"));
    networkProfileAction->setProperty("Url", QVariant::fromValue(netProfile->url));
    networkProfileAction->setProperty("IsRecursive", QVariant::fromValue(netProfile->isRecursive));
    networkProfileAction->setProperty("DocumentProfileIndex", QVariant::fromValue(netProfile->documentProfileIndex));
    networkProfileMenu->addAction(networkProfileAction);
    QObject::connect(networkProfileAction, &QAction::triggered, this, &MainWindow::openNetwork);
}

/**
 * Creates document entries in the cbDocumentProfiles
 *
 * @throws None
 */
void MainWindow::createDocumentEntries() {
    cbDocumentProfiles.clear();
    for (int i = 0; i < settings.documentProfileCount(); i++) {
        cbDocumentProfiles.addItem(QString::fromStdString(settings.DocumentProfile(i)->name), QVariant(i));
    }
    qDebug() << cbDocumentProfiles.count();
    // Set the document profile to the active document profile of the default network profile
    if (settings.networkProfileCount() > 0) {
        cbDocumentProfiles.setCurrentIndex(settings.NetworkProfile(0)->documentProfileIndex - 1);
    }
}

/**
 * Sets the tab order for the widgets in the main window.
 *
 * This function sets the tab order for the widgets in the main window by calling the `QWidget::setTabOrder` function.
 *
 * @throws None
 */
void MainWindow::setTabOrder() {
    QWidget::setTabOrder(&lsFiles, &leFileName);
    QWidget::setTabOrder(&leFileName, &pbRename);
    QWidget::setTabOrder(&pbRename, &leDestinationDir);
    QWidget::setTabOrder(&leDestinationDir, &pbDestinationDir);
}

/**
 * Sets the text for various UI elements in the MainWindow.
 *
 * @throws None
 */
void MainWindow::setText() {
    this->setWindowTitle(tr("Scan2OCR"));
    leFileName.setText(tr("pdf file name"));
    pbRename.setText(tr("&Rename"));
    lbDestinationDir.setText(tr("Target directory:"));
    leDestinationDir.setText(settings.DestinationDir());
    pbDestinationDir.setText("...");
}

/**
 * Calls the settings dialog after preparing the menu and toolbar.
 *
 * @throws None
 */
void MainWindow::settingsMenu() {
   SettingsUI settingsDialog;
   settingsDialog.showDialog();
   settings.readValues();

    //Update (clear) networkMenu and toolBar
    if (tbDefaultNetworkEntry.parent() != nullptr) {
        QAction* actionToRemove = toolBar.findChild<QAction*>(tbDefaultNetworkEntry.objectName());
        
        if (actionToRemove) {
            toolBar.removeAction(actionToRemove);
        }
        // Now remove all separators
        for (QAction *action : toolBar.findChildren<QAction*>()) {
            if (action->isSeparator()) {
                toolBar.removeAction(action);
            }
        }
        // If tbNetworkAction is used, remove it
        if (tbNetworkProfileAction) {
            toolBar.removeAction(tbNetworkProfileAction);
            tbNetworkProfileAction = nullptr;
        }
    }

    for (QAction *action : networkProfileMenu->actions()) {
        networkProfileMenu->removeAction(action);
    }
    networkProfileMenu->clear();
    
    for (int i=0; i < settings.networkProfileCount(); i++) {
        createNetworkMenuEntry(settings.NetworkProfile(i));
    }

    // Set the default document directory
    leDestinationDir.setText(settings.DestinationDir());
}

/**
 * Displays the About dialog for the application with the version information.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
void MainWindow::about() {
    QDialog aboutDialog;
    
    aboutDialog.setWindowTitle(tr("About Scan2OCR"));
    aboutDialog.setFixedWidth(640);
    aboutDialog.setFixedHeight(400);
 
    QVBoxLayout layout {&aboutDialog};
        
    QLabel lbImage {&aboutDialog};
    lbImage.setGeometry(QRect(0, 0, 640, 200));
    lbImage.setPixmap(QPixmap(":/images/writing.png"));
    
    QLabel lbText {&aboutDialog};
    
    QFont font;
    font.setPointSize(18);
    lbText.setFont(font);
    lbText.setText(QString("Scan2OCR Version ") + QCoreApplication::applicationVersion());   
    
    layout.addWidget(&lbImage);
    layout.addWidget(&lbText);
    layout.setAlignment(&lbText, Qt::AlignHCenter);
    
    aboutDialog.exec();
}

/**
 * Opens a file dialog to select a PDF file. If a file is selected, a new
 * ParseUrl object is created with the file path and newFileFound is called
 * with the new ParseUrl object and a document profile index of 0 (the default). Finally,
 * processFiles is called to process the newly found file.
 *
 * @return void
 *
 * @throws None
 */
void MainWindow::openFile(){
    QFileDialog FileDialog;
    QString File;

    File = FileDialog.getOpenFileName(this, tr("Open PDF file"), settings.DestinationDir(), tr("PDF files *.pdf"));
    if(File == ""){
        return;
    }

    // Create new Url for the selected file
    std::shared_ptr<ParseUrl>ptr_Url = std::make_shared<ParseUrl>("file://" + File.toStdString());
    
    // Process file
    newFileFound(std::move(ptr_Url), cbDocumentProfiles.currentIndex());
    processFiles();
}

/**
 * Opens a file dialog to select a directory path. If a valid path is selected, 
 * creates a new instance of Directory with the selected path, connects signals 
 * for new file found, and initializes the directory for processing files.
 *
 * @return void
 *
 * @throws None
 */
void MainWindow::openPath(){
 
    PathDialog pathDialog (this);
    pathDialog.setFileMode(QFileDialog::FileMode::Directory);
    pathDialog.setAcceptMode(QFileDialog::AcceptOpen);
    pathDialog.setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    pathDialog.setOption(QFileDialog::ShowDirsOnly, true);
    
    QString PathName {""};
    bool isRecursive {true};

    if (pathDialog.exec() == QDialog::Accepted) {
        PathName = pathDialog.selectedFiles().at(0);
        isRecursive = pathDialog.isRecursive();
    }
    else {
        return;
    }
    
    // Create Url for the selected directory
    ParseUrl url;
    url.Scheme ("file");
    url.Directory (PathName.toStdString());
    #ifdef DEBUG
        std::cout << "MainWindow::openPath() creating new instance of Directory " << url.Url() << " with parent_ptr to " << this << std::endl;
    #endif
    // Process directory
    p_Directory = std::make_shared<Directory> (url, this, isRecursive, cbDocumentProfiles.currentIndex());

    #ifdef DEBUG
        std::cout << "MainWindow::openPath() connecting foundNewFile at " << p_Directory.get() << " to newFileFound at" << this << std::endl;
    #endif
    QObject::connect(p_Directory.get(), &Directory::foundNewFile, this, &MainWindow::newFileFound, Qt::DirectConnection);
    p_Directory->initialize();
    processFiles();
}

/**
 * Opens a network directory and initializes the corresponding Directory object.
 *
 * @return void
 *
 * @throws None
 */
void MainWindow::openNetwork() {
    QObject *senderObject = QObject::sender();

    bool isRecursive {senderObject->property("IsRecursive").toBool()};
    ParseUrl url{senderObject->property("Url").value<ParseUrl>()};
    int documentProfileIndex {senderObject->property("DocumentProfileIndex").toInt()};

    #ifdef DEBUG
        std::cout << "MainWindow::openNetwork() creating new instance of Directory " << url.Url() << " with parent_ptr to " << this << std::endl;
    #endif

    p_Directory = std::make_shared<Directory> (url, this, isRecursive, documentProfileIndex);

    #ifdef DEBUG
        std::cout << "MainWindow::openNetwork() connecting foundNewFile at " << p_Directory.get() << " to newFileFound at" << this << std::endl;
    #endif
    QObject::connect(p_Directory.get(), &Directory::foundNewFile, this, &MainWindow::newFileFound, Qt::DirectConnection);
    p_Directory->initialize();
    processFiles();
}

/**
 * Adds a new PDF file to the list of PDF files in the MainWindow.
 *
 * @param ptr_Url a shared pointer to a ParseUrl object representing the URL of the PDF file
 * @param documentProfileIndex the index of the document profile to use for the new PDF file
 *
 * @throws None
 */
void MainWindow::newFileFound(std::shared_ptr<ParseUrl>ptr_Url, int documentProfileIndex) {
    #ifdef DEBUG
        std::cout << "MainWindow::newFileFound() on: " << ptr_Url->Url() << std::endl;
    #endif
    
    vec_pdfFiles.emplace_back(std::make_shared<PdfFile> (*(ptr_Url), this, documentProfileIndex));
}

/**
 * Processes the list of PDF files by initializing each file, adding its name to a list widget,
 * and setting the current row if needed.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
void MainWindow::processFiles() {
    for (auto &pdfFile : vec_pdfFiles) {
        pdfFile->initialize();
        lsFiles.addItem(QString::fromStdString(pdfFile->FileName()));

        if(lsFiles.currentRow() == -1) {
            lsFiles.setCurrentRow(0);
        }
    }
}

/**
 * Connects various signals and slots in the MainWindow.
 *
 * This function connects the signals and slots in the MainWindow to handle user interactions.
 * It connects the following signals and slots:
 * - When the user selects a file in the QListWidget lsFiles, the slot fileSelected() is called.
 * - When the Rename button pbRename is clicked, the slot renameToFinalName() is called.
 * - When the user presses Enter in the LineEdit leFileName, the slot renameToFinalName() is called.
 * - When the user clicks the button pbDestinationDir to open a QFileDialog, the slot getDestinationDir() is called.
 * - When the user changes the text in the LineEdit leDestinationDir, the slot directoryCompleter() is called.
 *
 * @throws None
 */
void MainWindow::connectSignals() {
    QObject::connect(&lsFiles, &QListWidget::itemSelectionChanged,
                      this, &MainWindow::fileSelected);
    QObject::connect(&pbRename, &QPushButton::clicked,
                      this, &MainWindow::renameToFinalName);
    QObject::connect(&leFileName, &QLineEdit::returnPressed,
                      this, &MainWindow::renameToFinalName);
    QObject::connect(&pbDestinationDir, &QPushButton::clicked,
                      this, &MainWindow::getDestinationDir);
    QObject::connect(&leDestinationDir, &QLineEdit::textChanged,
                      this, &MainWindow::directoryCompleter);
}
/**
 * Selects a file from the QListWidget lsFiles and displays the pdf.
 * Then the proposed file name is set in the LineEdit leFileName and
 * it is selected without the file extension
 *
 * @throws None
 */
void MainWindow::fileSelected(){
    int Index = lsFiles.currentIndex().row();
    if (Index < 0) {
        return;
    }

    #ifdef DEBUG
        std::cout << "MainWindow::fileSelected(), Index:" << Index << " on:" << vec_pdfFiles.at(Index)->FileName() << std::endl;
    #endif

    leFileName.setText(QString::fromStdString(vec_pdfFiles.at(Index)->FileName()));
    QString pdfFile = QString::fromStdString(vec_pdfFiles.at(Index)->pdfFileName());
    pdfDocument.load(pdfFile);
    
    pdfView.setDocument(&pdfDocument);
    vec_pdfFiles.at(Index)->returnFileContent()->close();

    leFileName.setFocus();

    // Select the part of the filename after the date until file extension to make renaming quicker
    int secondSpaceIndex = leFileName.text().indexOf(' ', leFileName.text().indexOf(' ') + 1);
    if(secondSpaceIndex != -1){
        leFileName.setSelection(secondSpaceIndex + 1, leFileName.text().length() - secondSpaceIndex - 5);
    }
}

/**
 * Updates the statusbar of the MainWindow.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
void MainWindow::statusUpdate() {

    if (vec_pdfFiles.size() > 0) {
        pbProgress.setMaximum (vec_pdfFiles.size() * vec_pdfFiles.front()->getStatusIncrement());
        pbProgress.value() < 0 ? pbProgress.setValue(1) : pbProgress.setValue(pbProgress.value() + 1);
    }    
    #ifdef DEBUG
        std::cout << "MainWindow::statusUpdate() from " << this << ", pbProgress: " << pbProgress.value() << std::endl;
        std::cout << "vec_pdfFiles.size(): " << vec_pdfFiles.size() << ", getStatusIncrement(): " << vec_pdfFiles.front()->getStatusIncrement() << ", pbProgress.maximum(): " << pbProgress.maximum() << std::endl;
    #endif
}

/**
 * Sets the maximum value of the progress bar to the size of vec_pdfFiles
 * and shows the progress bar.
 *
 * @return void
 */
void MainWindow::setMaxProgress(){
    pbProgress.setMaximum (vec_pdfFiles.size());
    pbProgress.show();
}

/**
 * Deletes the file at the current row in the QListWidget lsFiles.
 *
 * @return void
 *
 * @throws None
 */
void MainWindow::deleteFileSlot(){
    // Check if we know what to delete and get the index
    if (lsFiles.count() == 0) {
        return;
    }
    const int element = lsFiles.currentRow();
    deleteFile(element);
}

/**
 * Closes the main window and quits the application.
 *
 * @return void
 *
 * @throws None
 */
void MainWindow::cancel(){
    this->close();
    qApp->quit();
}

/**
 * Deletes the file at the specified index in the vector of PdfFiles and removes the corresponding item from the QListWidget.
 *
 * @param element The index of the file to delete in the vector of PdfFiles.
 *
 * @throws None
 */
void MainWindow::deleteFile (const int element) {
    if (vec_pdfFiles.at(element)) {
        // Remove scanned File
        vec_pdfFiles.at(element)->removeFile();
        
        // Remove the currently selected item from lsFiles
        lsFiles.takeItem(element);
        if (element > 0) lsFiles.setCurrentRow(element-1);
    }
}

/**
 * Renames the file to the final name and removes the corresponding item from the QListWidget.
 *
 * @return void
 *
 * @throws std::filesystem::filesystem_error if there are errors during file copying or removal.
 */
void MainWindow::renameToFinalName() {
    // Check if we know the final name
    if (lsFiles.count() == 0) {
        return;
    }

    // If the user has not terminated the Destination Directory with "/" then we should add the "/"
    std::string destinationDir = leDestinationDir.text().toStdString();
    if (destinationDir.substr(destinationDir.length() - 1) != "/") {
        destinationDir += "/";
    }

    // Check if the destination directory exists, if not create it
    if (!std::filesystem::exists(destinationDir)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Directory does not exist"), tr("This directory does not exist. Do you want to create it?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            try {
                std::filesystem::create_directory(destinationDir);
            } catch (std::filesystem::filesystem_error& e) {
                QMessageBox::critical(this, tr("Error while creating directory"), QString::fromStdString(e.what()));
            }
        }
    }

    const int element {lsFiles.currentRow()};
    
    bool retVal = vec_pdfFiles.at(element)->renameToFileName(destinationDir + leFileName.text().toStdString());
    if (retVal) {
        vec_pdfFiles.at(element)->removeFile();
        lsFiles.takeItem(element);
        (element > 0) ? lsFiles.setCurrentRow(element-1) : pdfDocument.close();
    }
    else {
        QMessageBox::critical(this, tr("Error while renaming file"), tr("Error while renaming file"));
    }
}

/**
 * Retrieves an existing directory from the user and sets it as the destination directory in the UI.
 *
 * @throws None
 */
void MainWindow::getDestinationDir(){
    QFileDialog PathDialog;
    QString destDir = PathDialog.getExistingDirectory(this, tr("Choose target directory"), settings.DestinationDir());
    leDestinationDir.setText(destDir);
}

/**
 * Generates a completer model for directories based on the given text.
 *
 * @param text The text to use as the base for directory search.
 *
 * @return None.
 *
 * @throws None.
 */
void MainWindow::directoryCompleter(const QString &text) {
    QStringList dirNames;
    std::string parentDir = text.toStdString();
    try
    {
        for (const auto& dir : std::filesystem::directory_iterator(parentDir)) {
            if (std::filesystem::is_directory(dir.path())) {
                dirNames.append(QString::fromStdString(dir.path().string() + "/"));
            }
        }
    }
    catch(const std::exception& e)
    {
        // Mainly because directories won't be found if the complete path name hasn't yet been typed into leDestinationDir
        return;
    }
    completer.setModel(new QStringListModel(dirNames, &completer));
}

/**
 * Hides the progress bar when all files are processed.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
void MainWindow::filesProcessed() {
    #ifdef DEBUG
        std::cout << "MainWindow::filesProcessed() from " << this << std::endl;
    #endif
    pbProgress.hide();
}

/**
 * Checks if the text content of a QLineEdit is the same as the default value.
 * If it is, sets the text color to red and returns false. Otherwise, returns true.
 *
 * @param LineEdit The QLineEdit to check.
 * @param Default The default value to compare against.
 *
 * @return True if the text content is different from the default value, false otherwise.
 */
bool::MainWindow::Modified(QLineEdit *LineEdit, const std::string Default){
    QPalette Warnfarbe;
    Warnfarbe.setColor(QPalette::Text, QColor(Qt::red));

    const std::string TextContent = LineEdit->text().toStdString();

    if (TextContent == Default) {
        LineEdit->setPalette(Warnfarbe);
        return false;
    }
    else {
        return true;
    }
}

/**
 * Sets the color of a QLineEdit to black.
 *
 * @throws None
 */
void MainWindow::TextToBlack () {
    QPalette Color;
    QLineEdit* leSender = qobject_cast<QLineEdit*>(sender());

    Color = leSender->palette();
    if (Color.text().color().value() == QColor(Qt::red).value()) {
            Color.setColor(QPalette::Text, QColor(Qt::black));
            leSender->setPalette(Color);
    }
    leSender->selectAll();
}


/**
 * Adds a custom widget to the PathDialog, which includes a checkbox to choose
 * whether subdirectories should be scanned.
 *
 * @throws None
 */
void PathDialog::addWidget() {
    setOption(QFileDialog::DontUseNativeDialog);
     QGridLayout* mainLayout = dynamic_cast <QGridLayout*>(this->layout());
        if(mainLayout){
            cbRecursive.setText(tr("Loop through subdirectories"));
            hbl.addWidget(&cbRecursive);
            int num_rows = mainLayout->rowCount();
            mainLayout->addLayout(&hbl, num_rows, 0, 1, -1);
        }
}

/**
 * A const member function that returns the state of the cbRecursive checkbox.
 *
 * @return the boolean value indicating whether the checkbox is checked
 *
 * @throws None
 */
bool PathDialog::isRecursive() const {
    return cbRecursive.isChecked();    
}

/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email ( at ) simonboettger . de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>
*/