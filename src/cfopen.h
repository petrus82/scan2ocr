#pragma once

#include "pdffile.h"
#include "parseurl.h"
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <QFileDialog>
#include <sys/socket.h>

QT_BEGIN_NAMESPACE

class clickableLineEdit : public QLineEdit {
    Q_OBJECT
public:
    clickableLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {};

    bool event (QEvent* ev) override;

signals:
    void clicked();
};

class Ui_fOpen
{
public:
    QLabel *lbImage;
    QPushButton *pbFile;
    QPushButton *pbPath;
    QPushButton *pbFinish;
    QPushButton *pbCancel;
    clickableLineEdit *leHost;
    clickableLineEdit *leUser;
    clickableLineEdit *leDirectory;
    clickableLineEdit *lePassword;
    clickableLineEdit *lePort;
    QFrame *lnLine;

    void setupUi(QWidget *fOpen)
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

    void retranslateUi(QWidget *fOpen)
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
    } // retranslateUi

};


namespace Ui {
    class fOpen: public Ui_fOpen {};
} // namespace Ui

QT_END_NAMESPACE

class PdfFileList;
class cfOpen : public QWidget, private Ui::fOpen
{
    Q_OBJECT

public:
    explicit cfOpen(QWidget *parent = nullptr);
    ~cfOpen();
    //QCompleter *completer = nullptr;
public slots:


private slots:
    void OpenFile();
    void OpenPath();
    void Finished();
    void Cancel();
    void DeleteText ();
    //void directoryCompleter(const QString  &text);

private:
    bool Modified(QLineEdit *LineEdit, const std::string Default);
    const std::string defaultTextHost {leHost->text().toStdString()};
    const std::string defaultTextDirectory {leDirectory->text().toStdString()};
    PdfFileList *pdfFileList;
};

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