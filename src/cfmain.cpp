#include "cfmain.h"
#include <QMetaMethod>
#include <QSettings>
#include <QStandardPaths>

Ui_mWindow::Ui_mWindow() {
    QSettings settings;
    destinationDir = settings.value("destinationDir",  QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)).toString();
}

void Ui_mWindow::setupUi(QMainWindow *mWindow) {
    mWindow->resize(1920, 1080);
    centralwidget = new QWidget(mWindow);
    lsFiles = new QListWidget(centralwidget);
    lsFiles->setGeometry(QRect(30, 80, 331, 971));
    
    leFileName = new QLineEdit(centralwidget);
    leFileName->setGeometry(QRect(430, 20, 431, 25));
    
    pbRename = new QPushButton(centralwidget);
    pbRename->setGeometry(QRect(910, 20, 101, 25));
    pbDelete = new QPushButton(centralwidget);
    pbDelete->setGeometry(QRect(30, 20, 101, 25));

    pdfView = new QPdfView(centralwidget);
    pdfView->setGeometry(QRect(410, 80, 1471, 970));
    pbProgress = new QProgressBar(centralwidget);
    pbProgress->setGeometry(QRect(430, 50, 581, 23));
    pbProgress->setVisible(true);
    pbProgress->setRange(0,1);
    pbProgress->setValue(0);
    lbDestinationDir = new QLabel(centralwidget);
    lbDestinationDir->setGeometry(QRect(1130, 20, 581, 23));
    leDestinationDir = new QLineEdit(centralwidget);
    
    leDestinationDir->setGeometry(QRect(1130, 50, 431, 25));
    pbDestinationDir = new QPushButton(centralwidget);
    pbDestinationDir->setGeometry(QRect(1561, 50, 25, 25));
    pbDestinationDir->setText("...");

    mWindow->setCentralWidget(centralwidget);

    retranslateUI(mWindow);

    QMetaObject::connectSlotsByName(mWindow);
    pbRename->setDefault(true);
    QWidget::setTabOrder(lsFiles, leFileName);
    QWidget::setTabOrder(leFileName, pbDelete);
    QWidget::setTabOrder(pbDelete, pbRename);
    QWidget::setTabOrder(pbRename, leDestinationDir);
    QWidget::setTabOrder(leDestinationDir, pbDestinationDir);
}

void Ui_mWindow::retranslateUI(QMainWindow *mWindow){
    mWindow->setWindowTitle(QCoreApplication::translate("mWindow", "Scan2OCR", nullptr));
    leFileName->setText(QCoreApplication::translate("mWindow", "pdf file name", nullptr));
    pbRename->setText(QCoreApplication::translate("mWindow", "&Rename", nullptr));
    pbDelete->setText(QCoreApplication::translate("mWindow", "&Delete", nullptr));
    lbDestinationDir->setText(QCoreApplication::translate("mWindow", "Target directory:", nullptr));
    leDestinationDir->setText(destinationDir);
}

cfMain::cfMain(QMainWindow *parent) : QMainWindow(parent)
{
    setupUi(this);
    
    pdfFileList = &PdfFileList::get_instance();
    pdfFileList->instance_cfMain(this);
    pdfDocument = new QPdfDocument(this);
    completer = new QCompleter(this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    leDestinationDir->setCompleter(completer);
    
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
    QObject::connect(pbDelete, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
}

cfMain::~cfMain()
{
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

void cfMain::loadPdf(const QString &fileName){
    pdfDocument->load(fileName);
    pdfView->setDocument(pdfDocument);
}

void cfMain::newFile(const std::string Filename) {
    lsFiles->addItem(QString::fromStdString(Filename));
    if(lsFiles->currentRow() == -1) {
        lsFiles->setCurrentRow(0);
    }
}

void cfMain::fileSelected(){
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

void cfMain::statusUpdate(){
    pbProgress->setMaximum (pdfFileList->maxStatus);
    pbProgress->setValue (pdfFileList->status());
}

void cfMain::setMaxProgress(){
    pbProgress->setMaximum (pdfFileList->maxStatus);
    pbProgress->show();
}

void cfMain::deleteFileSlot(){
    // Check if we know what to delete and get the index
    if (lsFiles->count() == 0) {
        return;
    }
    const int element = lsFiles->currentRow();
    deleteFile(element);
}

void cfMain::deleteFile (const int element) {
    // Disconnect SIGNAL statusChange
    QObject::disconnect(pdfFileList->pdfFile(element), SIGNAL(statusChange()), this, SLOT(statusUpdate()));

    // Remove scanned File
    pdfFileList->removeFile(element);
    
    // Remove the currently selected item from lsFiles
    lsFiles->takeItem(element);
    if (element > 0) lsFiles->setCurrentRow(element-1);
}

void cfMain::rename() {

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

void cfMain::getDestinationDir(){
    QFileDialog PathDialog;
    QString destDir = PathDialog.getExistingDirectory(this, tr("Choose target directory"), QString::fromStdString(constants::inputDir));
    leDestinationDir->setText(destDir);
}

void cfMain::directoryCompleter(const QString &text) {

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

void cfMain::processFinished() {
    pbProgress->hide();
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