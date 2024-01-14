# include "mainwindow.h"
#include <QMetaMethod>
#include <QSettings>
#include <QStandardPaths>
#include <QMessageBox>

/* #include <QResource>
#include <QDirIterator>
#include <QDebug> */

bool clickableLineEdit::event (QEvent* ev) {
    if (ev->type() == QEvent::MouseButtonPress) {
        emit clicked();
        return true;
    }

    // Make sure the rest of events are handled
    return QWidget::event(ev);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    
    pdfFileList = &PdfFileList::get_instance();
    pdfFileList->instance_cfMain(this);
    pdfDocument = new QPdfDocument(this);
    
    completer = new QCompleter(this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);

    createMenu();
    createOtherWidgets();
    setText();

    leDestinationDir->setCompleter(completer);
}

MainWindow::~MainWindow() {
    delete pdfDocument;
    delete completer;

    // disconnect all SIGNALS
    const QMetaObject* metaObject = this->metaObject();

    for (int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            QObject::disconnect(nullptr, SIGNAL(method), this, SLOT(nullptr));
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

    fileMenu = menuBar()->addMenu(tr("F&ile"));
    
    toolBar = new QToolBar(this);
    addToolBar(Qt::TopToolBarArea, toolBar);

/*     // Get resources
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << it.next();
    } */


    openFileAction = new QAction(tr("&Open File..."), this);
    openFileAction->setShortcuts(QKeySequence::Open);
    openFileAction->setStatusTip(tr("Open a single PDF file from local disk."));
    openFileAction->setIcon(QIcon(":/images/file.png"));
    connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile())); 
    fileMenu->addAction(openFileAction);
    toolBar->addAction(openFileAction);

    openPathAction = new QAction(tr("Open &Directory..."), this);
    openPathAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    openPathAction->setStatusTip(tr("Open a directory from local diskw."));
    openPathAction->setIcon(QIcon(":/images/directory.png"));
    connect(openPathAction, SIGNAL(triggered()), this, SLOT(openPath()));
    fileMenu->addAction(openPathAction);
    toolBar->addAction(openPathAction);

    networkProfileMenu = fileMenu->addMenu(tr("FTP &Server"));

    addProfileAction = new QAction(tr("Add &Profile..."), this);
    addProfileAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    addProfileAction->setStatusTip(tr("Add FTP server profiles."));
    connect(addProfileAction, SIGNAL(triggered()), this, SLOT(addProfile()));
    networkProfileMenu->addAction(addProfileAction);

    deleteProfileAction = new QAction(tr("Delete &Profile..."), this);
    deleteProfileAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    deleteProfileAction->setStatusTip(tr("Delete FTP server profiles."));
    connect(deleteProfileAction, SIGNAL(triggered()), this, SLOT(deleteProfile()));
    networkProfileMenu->addAction(deleteProfileAction);
    
    defaultProfileAction = new QAction(tr("Default &Profile..."), this);
    defaultProfileAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_P));
    defaultProfileAction->setStatusTip(tr("Set the default FTP server profile."));
    connect(defaultProfileAction, SIGNAL(triggered()), this, SLOT(defaultProfile()));
    networkProfileMenu->addAction(defaultProfileAction);

    networkProfileMenu->addSeparator();
    
    getNetworkProfiles();

    fileMenu->addSeparator();
    toolBar->addSeparator();

    renameAction = new QAction(tr("&Rename File"), this);
    renameAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
    renameAction->setIcon(QIcon(":/images/rename.png"));
    renameAction->setStatusTip(tr("Rename file to filename in destination directory."));
    connect(renameAction, SIGNAL(triggered()), this, SLOT(rename()));
    fileMenu->addAction(renameAction);
    toolBar->addAction(renameAction);

    deleteAction = new QAction(tr("De&lete File"), this);
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setIcon(QIcon(":/images/delete.png"));
    deleteAction->setStatusTip(tr("Delete file."));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(deleteFileSlot()));
    fileMenu->addAction(deleteAction);
    toolBar->addAction(deleteAction);

    fileMenu->addSeparator();
    
    settingsAction = new QAction(tr("Se&ttings..."), this);
    settingsAction->setShortcut(QKeySequence::Preferences);
    settingsAction->setStatusTip(tr("Settings."));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(settings()));
    fileMenu->addAction(settingsAction);

    fileMenu->addSeparator();

    cancelAction = new QAction(tr("&Quit"), this);
    cancelAction->setShortcut(QKeySequence::Quit);
    cancelAction->setStatusTip(tr("Quit."));
    connect(cancelAction, SIGNAL(triggered()), this, SLOT(cancel()));
    fileMenu->addAction(cancelAction);

    fileMenu->addSeparator();

    aboutAction = new QAction(tr("Abou&t"), this);
    aboutAction->setShortcut(QKeySequence::HelpContents);
    aboutAction->setStatusTip(tr("About."));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    fileMenu->addAction(aboutAction);
}

void MainWindow::getNetworkProfiles() {

    QSettings settings;
    settings.beginGroup("NetworkProfiles");
    
        QStringList profileKeys = settings.allKeys();

        for (auto &it : profileKeys) {
            qDebug() << it;
            ParseUrl url {settings.value(it).toString().toStdString()};
            createNetworkMenuEntry(url);
        }
    settings.endGroup();
}

void MainWindow::addProfile() {
    QDialog addProfile;
    QGridLayout layout(&addProfile);

    clickableLineEdit leHost;
    QLabel lbHost;
    lbHost.setText(tr("Host:"));

    clickableLineEdit leUser;
    QLabel lbUser;
    lbUser.setText(tr("Username:"));

    clickableLineEdit lePort;
    QLabel lbPort;
    lbPort.setText(tr("Port:"));

    clickableLineEdit leDirectory;
    QLabel lbDirectory;
    lbDirectory.setText(tr("Directory:"));

    clickableLineEdit lePassword;
    QLabel lbPassword;
    lbPassword.setText(tr("Password:"));

    QPushButton pbFinish;
    pbFinish.setText(tr("&OK"));
    pbFinish.setDefault(true);
    pbFinish.setFixedSize(80, 25);

    QPushButton pbCancel;
    pbCancel.setText(tr("&Cancel"));
    pbCancel.setFixedSize(80, 25);

    layout.setContentsMargins(20, 20, 20, 20);
    layout.setSpacing(20);
    layout.addWidget(&lbHost, 1, 1);
    layout.addWidget(&leHost, 1, 2);
    layout.addWidget(&lbPort, 2, 1);
    layout.addWidget(&lePort, 2, 2);
    layout.addWidget(&lbDirectory, 3, 1);
    layout.addWidget(&leDirectory, 3, 2);
    layout.addWidget(&lbUser, 4, 1);
    layout.addWidget(&leUser, 4, 2);
    layout.addWidget(&lbPassword, 5, 1);
    layout.addWidget(&lePassword, 5, 2);

    QSpacerItem spacer(0, 10);
    layout.addItem(&spacer, 6, 1, 1, 1);
    layout.addWidget(&pbFinish, 6, 1);
    layout.addWidget(&pbCancel, 6, 2);
    
    layout.setAlignment(&pbFinish, Qt::AlignHCenter);

    connect(&pbFinish, SIGNAL(clicked()), &addProfile, SLOT(accept()));
    connect(&pbCancel, SIGNAL(clicked()), &addProfile, SLOT(reject()));

    int result = addProfile.exec();

    if (result == QDialog::Accepted) {
        ParseUrl url;
        url.Scheme("sftp");
        url.Host(leHost.text().toStdString());
        url.Port(lePort.text().toInt());
        url.Directory(leDirectory.text().toStdString());
        url.Username(leUser.text().toStdString());
        url.Password(lePassword.text().toStdString());

        QSettings settings;

        settings.beginGroup("NetworkProfiles");
        // Get the number of entries in NetworkProfiles
        int maxEntries {settings.children().count()};
        QString keyName {QString::fromStdString(std::to_string(maxEntries + 1))};

        settings.setValue(keyName, url.qUrl());

        settings.endGroup();

        createNetworkMenuEntry(url);
    }
}

void MainWindow::deleteProfile() {
    QDialog dialogDelete;
    QHBoxLayout layout(&dialogDelete);
    QPushButton pbOK;
    pbOK.setText(tr("&OK"));
    pbOK.setFixedWidth(80);

    QPushButton pbCancel;
    pbCancel.setText(tr("&Cancel"));
    pbCancel.setFixedWidth(80);
    
    
    layout.addWidget(cbNetworkProfiles);
    layout.addWidget(&pbOK);
    layout.addWidget(&pbCancel);

    connect(&pbOK, SIGNAL(clicked()), &dialogDelete, SLOT(accept()));
    connect(&pbCancel, SIGNAL(clicked()), &dialogDelete, SLOT(reject()));

    dialogDelete.exec();
}

void MainWindow::defaultProfile () {
    
}

void MainWindow::createNetworkMenuEntry(ParseUrl &Url) {

        // Add Profile to NetworkProfileMenu
        QAction *networkProfileAction = new QAction(Url.Host().c_str(), this);
        networkProfileAction->setIcon(QIcon(":/images/network.png"));
        QVariant profileData = QVariant::fromValue(Url);
        networkProfileAction->setData(profileData);
        networkProfileMenu->addAction(networkProfileAction);
        connect(networkProfileAction, SIGNAL(triggered()), this, SLOT(openNetwork()));

        // Now the toolbar
        if (cbNetworkProfiles == nullptr) {
            // First we have to initialize the combobox
            cbNetworkProfiles = new QComboBox();
            cbNetworkProfiles->addItem(Url.Host().c_str());
            cbNetworkProfiles->setItemIcon(0, QIcon(":/images/network.png"));
            cbNetworkProfiles->setEditable(false);
            cbNetworkProfiles->setFixedWidth(250);
            toolBar->addWidget(cbNetworkProfiles);
            connect(cbNetworkProfiles, SIGNAL(clicked()), this, SLOT(openNetwork()));
        } 
        else {
            // Add Profile to ComboBox
            cbNetworkProfiles->addItem(Url.Host().c_str());
        }
}
void MainWindow::settings() {
    
}

void MainWindow::about() {
    QDialog aboutDialog;
    
    aboutDialog.setWindowTitle(tr("About Scan2OCR"));
    aboutDialog.setFixedWidth(640);
    aboutDialog.setFixedHeight(400);
 
    QVBoxLayout *layout = new QVBoxLayout(&aboutDialog);
        
    QLabel *lbImage = new QLabel(&aboutDialog);
    lbImage->setGeometry(QRect(0, 0, 640, 200));
    lbImage->setPixmap(QPixmap(":/images/writing.png"));
    
    QLabel *lbText = new QLabel(&aboutDialog);
    
    QFont font;
    font.setPointSize(18);
    lbText->setFont(font);
    lbText->setText(QString("Scan2OCR Version ") + QCoreApplication::applicationVersion());   
    
    layout->addWidget(lbImage);
    layout->addWidget(lbText);
    layout->setAlignment(lbText, Qt::AlignHCenter);
    
    aboutDialog.exec();
}

void MainWindow::openNetwork() {
    QAction *action = qobject_cast<QAction *>(sender());
    if(action) {
        QVariant profileData = action->data();
        ParseUrl url = profileData.value<ParseUrl>();
        qDebug() << "Url clicked: " << url.qUrl();
    }
}

void MainWindow::createOtherWidgets() {
    QWidget *centralWidget = new QWidget(this);
    QGridLayout *mainLayout = new QGridLayout;
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Create Instances
    lsFiles = new QListWidget(centralWidget);
    lsFiles->addItem("lsFiles");
    lsFiles->setMaximumWidth(400);
    leFileName = new QLineEdit(centralWidget);
    leFileName->setMaximumWidth(240);
    
    pbRename = new QPushButton(centralWidget);
    pbRename->setFixedWidth(120);
    pbRename->setDefault(true);

    pdfView = new QPdfView(centralWidget);
    
    leDestinationDir = new QLineEdit(centralWidget);
    lbDestinationDir = new QLabel(centralWidget);
    pbDestinationDir = new QPushButton(centralWidget);
    pbDestinationDir->setFixedWidth(25);
    
    pbProgress = new QProgressBar(centralWidget);
    
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
    mainLayout->setContentsMargins(10,10,10,10);

    mainLayout->addWidget(lbDestinationDir, 1, 3);
    mainLayout->addWidget(leFileName, 2, 1);
    mainLayout->addWidget(pbRename, 2, 2);
    mainLayout->addWidget(leDestinationDir, 2, 3);
    mainLayout->addWidget(pbDestinationDir, 2, 4);
    mainLayout->addWidget(lsFiles, 3, 1, 1, 2);
    mainLayout->addWidget(pdfView, 3, 3, 1, 2);
    mainLayout->addWidget(pbProgress, 4, 1, 1, 4);

    mainLayout->setColumnStretch(1, 1);
    mainLayout->setColumnStretch(2, 1);
    mainLayout->setColumnStretch(3, 2);
    mainLayout->setColumnStretch(4, 4);
    
}

void MainWindow::setTabOrder() {

    QWidget::setTabOrder(lsFiles, leFileName);
    QWidget::setTabOrder(leFileName, pbRename);
    QWidget::setTabOrder(pbRename, leDestinationDir);
    QWidget::setTabOrder(leDestinationDir, pbDestinationDir);
}

void MainWindow::setText() {
    this->setWindowTitle(tr("Scan2OCR"));
    leFileName->setText(tr("pdf file name"));
    pbRename->setText(tr("&Rename"));
    lbDestinationDir->setText(tr("Target directory:"));
    leDestinationDir->setText(destinationDir);
    pbDestinationDir->setText("...");
}

void MainWindow::connectSignals() {
    // If a new file is found
    QObject::connect(pdfFileList, SIGNAL(newFileAdded()), this, SLOT(setMaxProgress()), Qt::DirectConnection);  

    // If the file is processed
    QObject::connect(pdfFileList, SIGNAL(newFileComplete(const std::string)), this, SLOT(newFile(const std::string)), Qt::DirectConnection);

    // After all files have been processed
    QObject::connect(pdfFileList, SIGNAL(finishedProcessing()), this, SLOT(processFinished()));

    // If the user has selected a file
    QObject::connect(lsFiles, SIGNAL(itemSelectionChanged()), this, SLOT(fileSelected()));

    // If the Rename button has been clicked
    QObject::connect(pbRename, SIGNAL(clicked()), this, SLOT(rename()));

    // If enter has been pressed in the LineEdit leFileName
    QObject::connect(leFileName, SIGNAL(returnPressed()), this, SLOT(rename()));

    // If the user has clicked the button pbDestinationDir to open a QFileDialog
    QObject::connect(pbDestinationDir, SIGNAL(clicked()), this, SLOT(getDestinationDir()));

    // If the user has changed the text in the LineEdit leDestinationDir
    QObject::connect(leDestinationDir, SIGNAL(textChanged(const QString&)), this, SLOT(directoryCompleter(const QString&)));

    // If the user has clicked the button pbDelete
    //QObject::connect(pbDelete, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
}

void MainWindow::loadPdf(const QString &fileName){
    pdfDocument->load(fileName);
    pdfView->setDocument(pdfDocument);
}

void MainWindow::newFile(const std::string Filename) {
    lsFiles->addItem(QString::fromStdString(Filename));
    if(lsFiles->currentRow() == -1) {
        lsFiles->setCurrentRow(0);
    }
}

void MainWindow::fileSelected(){
    int Index = lsFiles->currentIndex().row();
    if (Index < 0) {
        return;
    }
    leFileName->setText(QString::fromStdString(pdfFileList->pdfFile(Index)->possibleFileName()));
    loadPdf(QString::fromStdString(pdfFileList->pdfFile(Index)->tempFileName()));
    leFileName->setFocus();

    // Select the part of the filename after the date until file extension to make rename quicker
    int secondSpaceIndex = leFileName->text().indexOf(' ', leFileName->text().indexOf(' ') + 1);
    if(secondSpaceIndex != -1){
        leFileName->setSelection(secondSpaceIndex + 1, leFileName->text().length() - secondSpaceIndex - 5);
    }
}

void MainWindow::statusUpdate(){
    pbProgress->setMaximum (pdfFileList->maxStatus);
    pbProgress->setValue (pdfFileList->status());
}

void MainWindow::setMaxProgress(){
    pbProgress->setMaximum (pdfFileList->maxStatus);
    pbProgress->show();
}

void MainWindow::deleteFileSlot(){
    // Check if we know what to delete and get the index
    if (lsFiles->count() == 0) {
        return;
    }
    const int element = lsFiles->currentRow();
    deleteFile(element);
}

void MainWindow::openFile(){
    QFileDialog FileDialog;
    QString File;

    File = FileDialog.getOpenFileName(this, tr("Open PDF file"), QString::fromStdString(constants::inputDir), tr("PDF files *.pdf"));
    if(File == ""){
        return;
    }

    this->setVisible(false);

    // Create new Url for the selected file
    ParseUrl url("file://" + File.toStdString());
    
    if(pdfFileList->addFiles(url)){
        this->close();
    }
    else{
        this->setVisible(true);
    }; 
}

void MainWindow::openPath(){
    QFileDialog PathDialog;
    QString PathName = PathDialog.getExistingDirectory(this, tr("Open directory"), QString::fromStdString(constants::inputDir));
    if(PathName == ""){
        return;
    }
    this->setVisible(false);

    // Create Url for the selected directory
    ParseUrl url;
    url.Scheme ("file");
    url.Directory (PathName.toStdString());

    if(pdfFileList->addFiles(url)){
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
    QObject::disconnect(pdfFileList->pdfFile(element), SIGNAL(statusChange()), this, SLOT(statusUpdate()));

    // Remove scanned File
    pdfFileList->removeFile(element);
    
    // Remove the currently selected item from lsFiles
    lsFiles->takeItem(element);
    if (element > 0) lsFiles->setCurrentRow(element-1);
}

void MainWindow::rename() {

    // Check if we know what to rename
    if (lsFiles->count() == 0) {
        return;
    }

    // If the user has not terminated the Destination Directory with "/" then we should add the "/"
    std::string destinationDir = leDestinationDir->text().toStdString();
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

    const int element {lsFiles->currentRow()};

    //Rename processed file to newFileName
    std::filesystem::path oldName = pdfFileList->pdfFile(element)->tempFileName();
    std::filesystem::path newName(destinationDir + leFileName->text().toStdString());

    if (std::filesystem::exists(newName)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Overwrite file?"), tr("This file already exists. Do you really want to overwrite ") + leFileName->text() + tr(" ?"),
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
    QString destDir = PathDialog.getExistingDirectory(this, tr("Choose target directory"), QString::fromStdString(constants::inputDir));
    leDestinationDir->setText(destDir);
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
    completer->setModel(new QStringListModel(dirNames, completer));
}

void MainWindow::processFinished() {
    pbProgress->hide();
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

/* void MainWindow::finished(){
    ParseUrl url;

    if (Modified(leHost, defaultTextHost)
        && Modified(leDirectory, defaultTextDirectory))
    {
        this->setVisible(false);

        url.Scheme ("sftp");

        // Check if Directory is terminated with "/"
        if (!leDirectory->text().endsWith("/")) {
            leDirectory->setText(leDirectory->text() + "/");
        }
        url.Directory (leDirectory->text().toStdString());
        url.Host (leHost->text().toStdString());
        url.Username (leUser->text().toStdString());
        url.Password (lePassword->text().toStdString());
        url.Port (lePort->text().toInt());

        if(pdfFileList->addFiles(url)) {
            this->close();
        } else {
            this->setVisible(true);
        }
    }
}
 */

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