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

    void setupUi(QMainWindow *mWindow);
    void retranslateUI(QMainWindow *mWindow); 
private:
    const QString destinationDir {""};
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