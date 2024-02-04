#include "mainwindow.h"
#include <QMetaMethod>
//#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    
    pdfFileList.instance_cfMain(this);
    
    completer.setCaseSensitivity(Qt::CaseInsensitive);
    completer.setCompletionMode(QCompleter::PopupCompletion);

    createMenu();
    createOtherWidgets();
    setText();

    leDestinationDir.setCompleter(&completer);
}

MainWindow::~MainWindow() {

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

    networkProfileMenu = fileMenu->addMenu(tr("FTP &Server"));

   // Get all network profile entries and create a menu entry for each of them
     for (auto it : settings.getNetworkProfiles()) {
        createNetworkMenuEntry(it);
    }

    fileMenu->addSeparator();
    toolBar.addSeparator();

    renameAction.setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    renameAction.setIcon(QIcon(":/images/rename.png"));
    renameAction.setStatusTip(tr("Rename file to filename in destination directory."));
    QObject::connect(&renameAction, &QAction::triggered, this, &MainWindow::rename);
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

    lsFiles.addItem("lsFiles");
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

void MainWindow::createNetworkMenuEntry(Settings::s_networkProfile &netProfile) {
        ParseUrl &Url {netProfile.url};
        const std::string profileName {netProfile.name};

        // Add Profile to NetworkProfileMenu
        networkProfileAction = std::make_unique<QAction>(Url.Host().c_str(), this);
        networkProfileAction->setIcon(QIcon(":/images/network.png"));
        QVariant profileData = QVariant::fromValue(Url);
        networkProfileAction->setData(profileData);
        networkProfileMenu->addAction(networkProfileAction.get());
        connect(networkProfileAction.get(), &QAction::triggered, this, &MainWindow::openNetwork);

        // Now the toolbar
        if (cbNetworkProfiles == nullptr) {
            // First we have to initialize the combobox
            cbNetworkProfiles = std::make_unique<QComboBox>();
            cbNetworkProfiles->addItem(profileName.c_str());
            cbNetworkProfiles->setItemIcon(0, QIcon(":/images/network.png"));
            cbNetworkProfiles->setEditable(false);
            cbNetworkProfiles->setFixedWidth(250);
            toolBar.addWidget(cbNetworkProfiles.get());
            connect(cbNetworkProfiles.get(), &QComboBox::activated, this, &MainWindow::openNetwork);
        } 
        else {
            // Add Profile to ComboBox
            cbNetworkProfiles->addItem(profileName.c_str());
        }
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
    leDestinationDir.setText(destinationDir);
    pbDestinationDir.setText("...");
}

void MainWindow::settingsMenu() {
   SettingsUI settingsDialog;
   settingsDialog.showDialog();

    //Update the networkMenu and the toolbar
    // if we have a cbNetworkProfiles clear it
    if (cbNetworkProfiles != nullptr) {
        cbNetworkProfiles->clear();
    }
    networkProfileMenu->clear();
    for (auto it : settings.getNetworkProfiles()) {
        createNetworkMenuEntry(it);
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

void MainWindow::openNetwork() {
    QObject *senderObject = QObject::sender();

    if (senderObject == cbNetworkProfiles.get()) {
        int index = cbNetworkProfiles->currentIndex();
        QAction *action = networkProfileMenu->actions()[index];
        ParseUrl url = action->data().value<ParseUrl>();
        
        pdfFileList.addFiles(url);
    }
    else if (senderObject == networkProfileAction.get()) {
        ParseUrl url = networkProfileAction->data().value<ParseUrl>();
        pdfFileList.addFiles(url);
    }
}
void MainWindow::connectSignals() {
    // If a new file is found
    QObject::connect(&pdfFileList.get_instance(), &PdfFileList::newFileAdded,
                      this, &MainWindow::setMaxProgress, Qt::DirectConnection);  

    // If the file is processed
    QObject::connect(&pdfFileList.get_instance(), &PdfFileList::newFileComplete,
                      this, &MainWindow::newFile);

    // After all files have been processed
    QObject::connect(&pdfFileList.get_instance(), &PdfFileList::finishedProcessing,
                      this, &MainWindow::processFinished);

    // If the user has selected a file
    QObject::connect(&lsFiles, &QListWidget::itemSelectionChanged,
                      this, &MainWindow::fileSelected);

    // If the Rename button has been clicked
    QObject::connect(&pbRename, &QPushButton::clicked,
                      this, &MainWindow::rename);

    // If enter has been pressed in the LineEdit leFileName
    QObject::connect(&leFileName, &QLineEdit::returnPressed,
                      this, &MainWindow::rename);

    // If the user has clicked the button pbDestinationDir to open a QFileDialog
    QObject::connect(&pbDestinationDir, &QPushButton::clicked,
                      this, &MainWindow::getDestinationDir);

    // If the user has changed the text in the LineEdit leDestinationDir
    QObject::connect(&leDestinationDir, &QLineEdit::textChanged,
                      this, &MainWindow::directoryCompleter);

    // If the user has clicked the button pbDelete
    //QObject::connect(pbDelete, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
}

void MainWindow::loadPdf(const QString &fileName){
    pdfDocument.load(fileName);
    pdfView.setDocument(&pdfDocument);
}

void MainWindow::newFile(const std::string Filename) {
    lsFiles.addItem(QString::fromStdString(Filename));
    if(lsFiles.currentRow() == -1) {
        lsFiles.setCurrentRow(0);
    }
}

void MainWindow::fileSelected(){
    int Index = lsFiles.currentIndex().row();
    if (Index < 0) {
        return;
    }
    leFileName.setText(QString::fromStdString(pdfFileList.pdfFile(Index)->possibleFileName()));
    loadPdf(QString::fromStdString(pdfFileList.pdfFile(Index)->tempFileName()));
    leFileName.setFocus();

    // Select the part of the filename after the date until file extension to make rename quicker
    int secondSpaceIndex = leFileName.text().indexOf(' ', leFileName.text().indexOf(' ') + 1);
    if(secondSpaceIndex != -1){
        leFileName.setSelection(secondSpaceIndex + 1, leFileName.text().length() - secondSpaceIndex - 5);
    }
}

void MainWindow::statusUpdate(){
    pbProgress.setMaximum (pdfFileList.maxStatus);
    pbProgress.setValue (pdfFileList.status());
}

void MainWindow::setMaxProgress(){
    pbProgress.setMaximum (pdfFileList.maxStatus);
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

void MainWindow::openFile(){
    QFileDialog FileDialog;
    QString File;

    File = FileDialog.getOpenFileName(this, tr("Open PDF file"), QString::fromStdString(settings.getDestinationDir()), tr("PDF files *.pdf"));
    if(File == ""){
        return;
    }

    this->setVisible(false);

    // Create new Url for the selected file
    ParseUrl url("file://" + File.toStdString());
    
    if(pdfFileList.addFiles(url)){
        this->close();
    }
    else{
        this->setVisible(true);
    }; 
}

void MainWindow::openPath(){
    QFileDialog PathDialog;
    QString PathName = PathDialog.getExistingDirectory(this, tr("Open directory"), QString::fromStdString(settings.getDestinationDir()));
    if(PathName == ""){
        return;
    }
    this->setVisible(false);

    // Create Url for the selected directory
    ParseUrl url;
    url.Scheme ("file");
    url.Directory (PathName.toStdString());

    if(pdfFileList.addFiles(url)){
        this->close();
    } else {
        this->setVisible(true);
    }
}

void MainWindow::cancel(){
    this->close();
    qApp->quit();
}

void MainWindow::deleteFile (const int element) {
    // Disconnect SIGNAL statusChange
        QObject::disconnect(&(*pdfFileList.pdfFile(element)), &PdfFile::statusChange, this, &MainWindow::statusUpdate);

    // Remove scanned File
    pdfFileList.removeFile(element);
    
    // Remove the currently selected item from lsFiles
    lsFiles.takeItem(element);
    if (element > 0) lsFiles.setCurrentRow(element-1);
}

void MainWindow::rename() {

    // Check if we know what to rename
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

    //Rename processed file to newFileName
    std::filesystem::path oldName = pdfFileList.pdfFile(element)->tempFileName();
    std::filesystem::path newName(destinationDir + leFileName.text().toStdString());

    if (std::filesystem::exists(newName)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Overwrite file?"), tr("This file already exists. Do you really want to overwrite ") + leFileName.text() + tr(" ?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    try {
        std::ifstream oldFile(oldName, std::ios::binary);
        std::ofstream newFile(newName, std::ios::binary);
        newFile << oldFile.rdbuf();
        oldFile.close();
        newFile.close();
        std::uintmax_t sizeOldFile = std::filesystem::file_size(oldName);
        std::uintmax_t sizeNewFile = std::filesystem::file_size(newName);
        
        if (sizeOldFile == sizeNewFile) {
            std::filesystem::remove(oldName);
        }
        else {
            QMessageBox::critical(this, tr("Error while renaming"), tr("This file cannot be renamed."));
        }
    } catch (std::filesystem::filesystem_error& e) {
        QMessageBox::critical(this, tr("Error while renaming"), QString::fromStdString(e.what()));
    }

    // Now delete all temp files and the source files and clean up the UI
    deleteFile(element);

}

void MainWindow::getDestinationDir(){
    QFileDialog PathDialog;
    QString destDir = PathDialog.getExistingDirectory(this, tr("Choose target directory"), QString::fromStdString(settings.getDestinationDir()));
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
        // Mainly because directories won't be found if not the complete name has been typed in
        return;
    }
    completer.setModel(new QStringListModel(dirNames, &completer));
}

void MainWindow::processFinished() {
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

/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email (at) simonboettger.der

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