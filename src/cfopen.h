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

    void setupUi(QWidget *fOpen);
    void retranslateUi(QWidget *fOpen);
};


namespace Ui {
    class fOpen: public Ui_fOpen {};
}

QT_END_NAMESPACE

class PdfFileList;
class cfOpen : public QWidget, private Ui::fOpen
{
    Q_OBJECT

public:
    explicit cfOpen(QWidget *parent = nullptr);
    ~cfOpen();
public slots:


private slots:
    void OpenFile();
    void OpenPath();
    void Finished();
    void Cancel();
    void DeleteText ();

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