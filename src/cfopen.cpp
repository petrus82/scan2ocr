#include "cfopen.h"
void Ui_fOpen::setupUi(QWidget *fOpen)
    {
        Qt::WindowFlags flags = Qt::FramelessWindowHint;
        fOpen->setWindowFlags(flags);
        fOpen->resize(640, 480);
        fOpen->setWindowModality(Qt::ApplicationModal);
        QPoint point;
        point.rx()=640;
        point.ry()=300;
        fOpen->move(point);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(fOpen->sizePolicy().hasHeightForWidth());
        fOpen->setSizePolicy(sizePolicy);
        fOpen->setContextMenuPolicy(Qt::NoContextMenu);
        lbImage = new QLabel(fOpen);
        lbImage->setGeometry(QRect(0, 0, 640, 200));
        lbImage->setPixmap(QPixmap(":/images/writing.png"));

        pbFile = new QPushButton(fOpen);
        pbFile->setGeometry(QRect(40, 270, 130, 25));
        pbPath = new QPushButton(fOpen);
        pbPath->setGeometry(QRect(40, 330, 130, 25));
        
        
        lnLine = new QFrame(fOpen);
        lnLine->setGeometry(QRect(200, 260, 20, 201));
        lnLine->setFrameShape(QFrame::VLine);
        lnLine->setFrameShadow(QFrame::Sunken);
        
        leHost = new clickableLineEdit(fOpen);
        leHost->setGeometry(QRect(250, 270, 150, 25));
        leUser = new clickableLineEdit(fOpen);
        leUser->setGeometry(QRect(250, 320, 150, 25));
        lePort = new clickableLineEdit(fOpen);
        lePort->setGeometry(QRect(250, 370, 150, 25));
        
        leDirectory = new clickableLineEdit(fOpen);
        leDirectory->setGeometry(QRect(450, 270, 150, 25));
        lePassword = new clickableLineEdit(fOpen);
        lePassword->setGeometry(QRect(450, 320, 150, 25));
        pbFinish = new QPushButton(fOpen);
        pbFinish->setGeometry(QRect(450, 370, 80, 25));
        pbFinish->setDefault(true);

        pbCancel = new QPushButton(fOpen);
        pbCancel->setGeometry(QRect(620,0,20,20));
        
        QWidget::setTabOrder(pbFile, pbPath);
        QWidget::setTabOrder(pbPath, leHost);
        QWidget::setTabOrder(leHost, leDirectory);
        QWidget::setTabOrder(leDirectory, leUser);
        QWidget::setTabOrder(leUser, lePassword);
        QWidget::setTabOrder(lePassword, lePort);

        retranslateUi(fOpen);

        QMetaObject::connectSlotsByName(fOpen);
}

void Ui_fOpen::retranslateUi(QWidget *fOpen)
    {
        fOpen->setWindowTitle(QCoreApplication::translate("fOpen", "Scan2OCR", nullptr));
        pbFile->setText(QCoreApplication::translate("fOpen", "Open &file", nullptr));
        pbPath->setText(QCoreApplication::translate("fOpen", "Open &directory", nullptr));
        pbFinish->setText(QCoreApplication::translate("fOpen", "F&inished", nullptr));
        leHost->setText(QCoreApplication::translate("fOpen", "FTP Server", nullptr));
        leUser->setText(QCoreApplication::translate("fOpen", "Username", nullptr));
        leDirectory->setText(QCoreApplication::translate("fOpen", "FTP directory", nullptr));
        lePassword->setText(QCoreApplication::translate("fOpen", "Password", nullptr));
        lePort->setText(QCoreApplication::translate("fOpen", "Port", nullptr));
        pbCancel->setText("x");
    }



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

void cfOpen::OpenPath(){
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