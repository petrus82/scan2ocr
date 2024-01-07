#pragma once

#include "pdffile.h"
#include "parseurl.h"
#include <QObject>
#include <QFileDialog>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>
#include <QtWidgets/QProgressBar>
#include <QPushButton>
#include <QLabel>
#include <QPdfDocument>
#include <QCompleter>
#include <QStringListModel>
#include <qpdfview.h>

QT_BEGIN_NAMESPACE

class Ui_mWindow
{
public:
    QWidget *centralwidget;
    QListWidget *lsFiles;
    QLineEdit *leFileName;
    QPushButton *pbRename;
    QPushButton *pbDelete;
    QPdfView *pdfView;
    QProgressBar *pbProgress;
    QLineEdit *leDestinationDir;
    QLabel *lbDestinationDir;
    QPushButton *pbDestinationDir;

    void setupUi(QMainWindow *mWindow)
    {
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

        retranslateUi(mWindow);

        QMetaObject::connectSlotsByName(mWindow);
        pbRename->setDefault(true);
        QWidget::setTabOrder(lsFiles, leFileName);
        QWidget::setTabOrder(leFileName, pbDelete);
        QWidget::setTabOrder(pbDelete, pbRename);
        QWidget::setTabOrder(pbRename, leDestinationDir);
        QWidget::setTabOrder(leDestinationDir, pbDestinationDir);
    }

    void retranslateUi(QMainWindow *mWindow)
    {
        mWindow->setWindowTitle(QCoreApplication::translate("mWindow", "Scan2OCR", nullptr));
        leFileName->setText(QCoreApplication::translate("mWindow", "pdf file name", nullptr));
        pbRename->setText(QCoreApplication::translate("mWindow", "&Umbenennen", nullptr));
        pbDelete->setText(QCoreApplication::translate("mWindow", "&Löschen", nullptr));
        lbDestinationDir->setText(QCoreApplication::translate("mWindow", "Zielverzeichnis:", nullptr));
        leDestinationDir->setText("/home/simon/Daten/Dokumente");
    } // retranslateUi
private:

};

namespace Ui {
    class mWindow: public Ui_mWindow {};
} // namespace Ui

QT_END_NAMESPACE


class PdfFileList;
class cfMain : public QMainWindow, private Ui::mWindow
{
    Q_OBJECT

public:
    explicit cfMain(QMainWindow *parent = nullptr);
    ~cfMain();   

public slots:
void loadPdf(const QString &fileName);
void newFile(const std::string Filename);
void statusUpdate();
void processFinished();
void setMaxProgress();
void rename();
void getDestinationDir();
void deleteFileSlot();

private slots:
void fileSelected();
void directoryCompleter(const QString  &text);

private:
QPdfDocument *pdfDocument = nullptr;
QCompleter *completer = nullptr;
void deleteFile (const int element);

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