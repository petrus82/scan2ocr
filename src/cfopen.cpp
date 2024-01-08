//#include "scan2ocr.h"
#include "cfopen.h"

cfOpen::cfOpen(QWidget *parent) : QWidget(parent)
{
    setupUi(this);
    pdfFileList = &PdfFileList::get_instance();

    QObject::connect(pbFile, SIGNAL(clicked()), this, SLOT(OpenFile()));
    QObject::connect(pbPath, SIGNAL(clicked()), this, SLOT(OpenPath()));
    QObject::connect(pbCancel, SIGNAL(clicked()), this, SLOT(Cancel()));
    QObject::connect(pbFinish, SIGNAL(clicked()), this, SLOT(Finished()));

    QObject::connect(leHost, SIGNAL(clicked()), this, SLOT(DeleteText()));
    QObject::connect(leHost, SIGNAL(returnPressed()), this, SLOT(Finished()));

    QObject::connect(leDirectory, SIGNAL(clicked()), this, SLOT(DeleteText()));
    QObject::connect(leDirectory, SIGNAL(returnPressed()), this, SLOT(Finished()));

    QObject::connect(leUser, SIGNAL(clicked()), this, SLOT(DeleteText()));
    QObject::connect(leUser, SIGNAL(returnPressed()), this, SLOT(Finished()));

    QObject::connect(lePassword, SIGNAL(clicked()), this, SLOT(DeleteText()));
    QObject::connect(lePassword, SIGNAL(returnPressed()), this, SLOT(Finished()));

    QObject::connect(lePort, SIGNAL(clicked()), this, SLOT(DeleteText()));
    QObject::connect(lePort, SIGNAL(returnPressed()), this, SLOT(Finished()));
}

cfOpen::~cfOpen() {
    // disconnect all SIGNALS
    const QMetaObject* metaObject = this->metaObject();

    for (int i = metaObject->methodOffset(); i < metaObject->methodCount(); ++i) {
        QMetaMethod method = metaObject->method(i);
        if (method.methodType() == QMetaMethod::Signal) {
            QObject::disconnect(nullptr, SIGNAL(method), this, SLOT(nullptr));
        }
    }
}

void cfOpen::OpenFile(){
    QFileDialog FileDialog;
    QString File;

    File = FileDialog.getOpenFileName(this, tr("Open PDF file"), QString(constants::inputDir), tr("PDF files *.pdf"));
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

void cfOpen::OpenPath(){
    QFileDialog PathDialog;
    QString PathName = PathDialog.getExistingDirectory(this, tr("Open directory"), QString(constants::inputDir));
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

void cfOpen::Cancel(){
    this->close();
    qApp->quit();
}

bool cfOpen::Modified(QLineEdit *LineEdit, const std::string Default){
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

void cfOpen::Finished(){
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

void cfOpen::DeleteText () {
    QPalette Color;
    QLineEdit* leSender = qobject_cast<QLineEdit*>(sender());

    Color = leSender->palette();
    if (Color.text().color().value() == QColor(Qt::red).value()) {
            Color.setColor(QPalette::Text, QColor(Qt::black));
            leSender->setPalette(Color);
    }
    leSender->selectAll();
}

bool clickableLineEdit::event (QEvent* ev) {
    if (ev->type() == QEvent::MouseButtonPress) {
        emit clicked();
        return true;
    }

    // Make sure the rest of events are handled
    return QWidget::event(ev);
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