#include "mainwindow.h"
#include <QMetaMethod>
#include <QStandardPaths>
#include <QMessageBox>

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

void MainWindow::createMenu() {
    /*
    Menu structure:
    File    - Open File         : openFile()
            - Open Directory    : openPath()
            - Open FTP Server   - Add Profile   : addProfile()
                                - Profile 1     : openNetwork(1)
                                ....
                                - Profile n     : openNetwork(n)
            - Rename File       : rename()
            - Delete File       : deleteFile()
            - Settings          : settings()
            - Quit              : cancel()
    Help    - About             : About()
    */
    
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

    renameAction.setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    renameAction.setIcon(QIcon(":/images/rename.png"));
    renameAction.setStatusTip(tr("Rename file to filename in destination directory."));
    QObject::connect(&renameAction, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(&renameAction);
    toolBar.addAction(&renameAction);

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
void MainWindow::createOtherWidgets() {
    centralWidget.setLayout(&mainLayout);
    setCentralWidget(&centralWidget);

    lsFiles.setMaximumWidth(400);
    leFileName.setMaximumWidth(240);
    
    pbRename.setFixedWidth(120);
    pbRename.setDefault(true);

    pbDestinationDir.setFixedWidth(25);
    
    // Create Layout
    // add widgets to layout (row, col, rowspan, colspan)
    /*
      |c1           |c2         |c3                |c4              |
    -----------------------------------------------------------------
    r0: Menu
    -----------------------------------------------------------------
    r0:Toolbar
    -----------------------------------------------------------------
    r1:                         | lbDestinationDir
    -----------------------------------------------------------------                            
    r2:|leFilename  | pbRename  | leDestinationDir | pbDestinationDir
    -----------------------------------------------------------------
    r3:|        lsFiles         |               pdfView              |
    ------------------------------------------------------------------
    r4: pbProgress
    ------------------------------------------------------------------
    */
    mainLayout.setContentsMargins(10,10,10,10);

    mainLayout.addWidget(&lbDestinationDir, 1, 3);
    mainLayout.addWidget(&leFileName, 2, 1);
    mainLayout.addWidget(&pbRename, 2, 2);
    mainLayout.addWidget(&leDestinationDir, 2, 3);
    mainLayout.addWidget(&pbDestinationDir, 2, 4);
    mainLayout.addWidget(&lsFiles, 3, 1, 1, 2);
    mainLayout.addWidget(&pdfView, 3, 3, 1, 2);
    mainLayout.addWidget(&pbProgress, 4, 1, 1, 4);

    mainLayout.setColumnStretch(1, 1);
    mainLayout.setColumnStretch(2, 1);
    mainLayout.setColumnStretch(3, 2);
    mainLayout.setColumnStretch(4, 4);
    
}

void MainWindow::createNetworkMenuEntry(Settings::networkProfile *netProfile) {
    // if it is the default network profile, add it to the toolbar
    if (netProfile->isDefault) {
        toolBar.addSeparator();
        tbDefaultNetworkEntry.setIcon(QIcon(":/images/network.png"));
        tbDefaultNetworkEntry.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        tbDefaultNetworkEntry.setText(netProfile->name.c_str());
        tbDefaultNetworkEntry.setProperty("Url", QVariant::fromValue(netProfile->url));
        toolBar.addWidget(&tbDefaultNetworkEntry);
        QObject::connect(&tbDefaultNetworkEntry, &QToolButton::clicked, this, &MainWindow::openNetwork); 
            
        // if there are more entries, create a drop down menu
        if (settings.networkProfileCount() > 1) {
            tbNetworkProfiles.setIcon(QIcon(":/images/down_arrow.svg"));
            tbNetworkProfiles.setFixedWidth(10);
            tbDefaultNetworkEntry.setFixedWidth(networkProfileMenu->width()+ 10);
            toolBar.addWidget(&tbNetworkProfiles);
            connect(&tbNetworkProfiles, &QToolButton::clicked, [&]() {
                QPoint menuPos = tbDefaultNetworkEntry.mapToGlobal(QPoint(0, tbDefaultNetworkEntry.height()));
                networkProfileMenu->exec(menuPos);
            });
        }
        toolBar.addSeparator();
    }
    // Add Profile to NetworkProfileMenu
    QAction *networkProfileAction = new QAction(netProfile->name.c_str(), this);
    networkProfileAction->setIcon(QIcon(":/images/network.png"));
    QVariant profileData = QVariant::fromValue(netProfile->url);
    networkProfileAction->setData(profileData);
    networkProfileMenu->addAction(networkProfileAction);
    QObject::connect(networkProfileAction, &QAction::triggered, this, &MainWindow::openNetwork);
}

void MainWindow::setTabOrder() {
    QWidget::setTabOrder(&lsFiles, &leFileName);
    QWidget::setTabOrder(&leFileName, &pbRename);
    QWidget::setTabOrder(&pbRename, &leDestinationDir);
    QWidget::setTabOrder(&leDestinationDir, &pbDestinationDir);
}

void MainWindow::setText() {
    this->setWindowTitle(tr("Scan2OCR"));
    leFileName.setText(tr("pdf file name"));
    pbRename.setText(tr("&Rename"));
    lbDestinationDir.setText(tr("Target directory:"));
    leDestinationDir.setText(settings.DestinationDir());
    pbDestinationDir.setText("...");
}

void MainWindow::settingsMenu() {
   SettingsUI settingsDialog;
   settingsDialog.showDialog();

    //Update (clear) networkMenu and toolBar
    if (tbDefaultNetworkEntry.parent() != nullptr) {
        QAction* actionToRemove = toolBar.findChild<QAction*>(tbDefaultNetworkEntry.objectName());
        if (actionToRemove) {
            toolBar.removeAction(actionToRemove);
        }
    }

    for (QAction *action : networkProfileMenu->actions()) {
        networkProfileMenu->removeAction(action);
    }
    networkProfileMenu->clear();
    
    for (int i=0; i < settings.networkProfileCount(); i++) {
        createNetworkMenuEntry(settings.NetworkProfile(i));
    }
}

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
    newFileFound(std::move(ptr_Url));
    processFiles();
}

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
    p_Directory = std::make_shared<Directory> (url, this, isRecursive);

    #ifdef DEBUG
        std::cout << "MainWindow::openPath() connecting foundNewFile at " << p_Directory.get() << " to newFileFound at" << this << std::endl;
    #endif
    QObject::connect(p_Directory.get(), &Directory::foundNewFile, this, &MainWindow::newFileFound, Qt::DirectConnection);
    p_Directory->initialize();
    processFiles();
}

void MainWindow::openNetwork() {
    QObject *senderObject = QObject::sender();
    ParseUrl url;
    bool isRecursive {true};

    if (senderObject == static_cast<QObject*>(&tbDefaultNetworkEntry)) {
        url = tbDefaultNetworkEntry.property("Url").value<ParseUrl>();
    }
    if (senderObject == networkProfileAction.get()) {
        url = networkProfileAction->data().value<ParseUrl>();
    }
    #ifdef DEBUG
        std::cout << "MainWindow::openNetwork() creating new instance of Directory " << url.Url() << " with parent_ptr to " << this << std::endl;
    #endif
    p_Directory = std::make_shared<Directory> (url, this, isRecursive);

    #ifdef DEBUG
        std::cout << "MainWindow::openNetwork() connecting foundNewFile at " << p_Directory.get() << " to newFileFound at" << this << std::endl;
    #endif
    QObject::connect(p_Directory.get(), &Directory::foundNewFile, this, &MainWindow::newFileFound, Qt::DirectConnection);
    p_Directory->initialize();
    processFiles();
}

void MainWindow::newFileFound(std::shared_ptr<ParseUrl>ptr_Url) {
    #ifdef DEBUG
        std::cout << "MainWindow::newFileFound() on: " << ptr_Url->Url() << std::endl;
    #endif
    
    vec_pdfFiles.emplace_back(std::make_shared<PdfFile> (*(ptr_Url), this));
}

void MainWindow::processFiles() {
    for (auto &pdfFile : vec_pdfFiles) {
        pdfFile->initialize();
        lsFiles.addItem(QString::fromStdString(pdfFile->FileName()));

        if(lsFiles.currentRow() == -1) {
            lsFiles.setCurrentRow(0);
        }
    }
}

void MainWindow::connectSignals() {
    // If the user has selected a file
    QObject::connect(&lsFiles, &QListWidget::itemSelectionChanged,
                      this, &MainWindow::fileSelected);

    // If the Rename button has been clicked
    QObject::connect(&pbRename, &QPushButton::clicked,
                      this, &MainWindow::save);

    // If enter has been pressed in the LineEdit leFileName
    QObject::connect(&leFileName, &QLineEdit::returnPressed,
                      this, &MainWindow::save);

    // If the user has clicked the button pbDestinationDir to open a QFileDialog
    QObject::connect(&pbDestinationDir, &QPushButton::clicked,
                      this, &MainWindow::getDestinationDir);

    // If the user has changed the text in the LineEdit leDestinationDir
    QObject::connect(&leDestinationDir, &QLineEdit::textChanged,
                      this, &MainWindow::directoryCompleter);
}
void MainWindow::fileSelected(){
    int Index = lsFiles.currentIndex().row();
    if (Index < 0) {
        return;
    }

    #ifdef DEBUG
        std::cout << "MainWindow::fileSelected(), Index:" << Index << " on:" << vec_pdfFiles.at(Index)->FileName() << std::endl;
    #endif

    leFileName.setText(QString::fromStdString(vec_pdfFiles.at(Index)->FileName()));

    pdfDocument.load(vec_pdfFiles.at(Index)->returnFileContent().get());
    pdfView.setDocument(&pdfDocument);
    vec_pdfFiles.at(Index)->returnFileContent()->close();

    leFileName.setFocus();

    // Select the part of the filename after the date until file extension to make renaming quicker
    int secondSpaceIndex = leFileName.text().indexOf(' ', leFileName.text().indexOf(' ') + 1);
    if(secondSpaceIndex != -1){
        leFileName.setSelection(secondSpaceIndex + 1, leFileName.text().length() - secondSpaceIndex - 5);
    }
}

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

void MainWindow::setMaxProgress(){
    pbProgress.setMaximum (vec_pdfFiles.size());
    pbProgress.show();
}

void MainWindow::deleteFileSlot(){
    // Check if we know what to delete and get the index
    if (lsFiles.count() == 0) {
        return;
    }
    const int element = lsFiles.currentRow();
    deleteFile(element);
}

void MainWindow::cancel(){
    this->close();
    qApp->quit();
}

void MainWindow::deleteFile (const int element) {
    if (vec_pdfFiles.at(element)) {
        // Remove scanned File
        vec_pdfFiles.at(element)->removeFile();
        
        // Remove the currently selected item from lsFiles
        lsFiles.takeItem(element);
        if (element > 0) lsFiles.setCurrentRow(element-1);
    }
}

void MainWindow::save() {
    // Check if we know what to save
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

    //Save pdf to disk and remove element if no error
    bool retVal = vec_pdfFiles.at(element)->saveToFile(destinationDir + leFileName.text().toStdString());
    if (retVal) {
        vec_pdfFiles.at(element)->removeFile();
        lsFiles.takeItem(element);
        (element > 0) ? lsFiles.setCurrentRow(element-1) : pdfDocument.close();
    }
}

void MainWindow::getDestinationDir(){
    QFileDialog PathDialog;
    QString destDir = PathDialog.getExistingDirectory(this, tr("Choose target directory"), settings.DestinationDir());
    leDestinationDir.setText(destDir);
}

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

void MainWindow::filesProcessed() {
    #ifdef DEBUG
        std::cout << "MainWindow::filesProcessed() from " << this << std::endl;
    #endif
    pbProgress.hide();
}

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

void MainWindow::deleteText () {
    QPalette Color;
    QLineEdit* leSender = qobject_cast<QLineEdit*>(sender());

    Color = leSender->palette();
    if (Color.text().color().value() == QColor(Qt::red).value()) {
            Color.setColor(QPalette::Text, QColor(Qt::black));
            leSender->setPalette(Color);
    }
    leSender->selectAll();
}

// Custom PathDialog with Checkbox to choose if subdirectories should be scanned
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