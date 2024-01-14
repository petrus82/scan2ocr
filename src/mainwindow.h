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
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QPdfDocument>
#include <QCompleter>
#include <QStringListModel>
#include <QLayout>
#include <QMenuBar>
#include <QToolBar>
#include <qpdfview.h>

QT_BEGIN_NAMESPACE

class clickableLineEdit : public QLineEdit {
    Q_OBJECT
public:
    clickableLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {};

    bool event (QEvent* ev) override;

signals:
    void clicked();
};

class MainWindow : public QMainWindow {

Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QGridLayout *mainLayout;
    QWidget *centralWidget;
    QListWidget *lsFiles;
    QLineEdit *leFileName;
    QPushButton *pbRename;
    //QPushButton *pbDelete;
    QPdfView *pdfView;
    QProgressBar *pbProgress;
    QLineEdit *leDestinationDir;
    QLabel *lbDestinationDir;
    QPushButton *pbDestinationDir;
    QComboBox *cbNetworkProfiles;

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
    void openFile();
    void openPath();
    void openNetwork();
    void addProfile();
    void deleteProfile();
    void defaultProfile();
    void settings();
    void cancel();
    void about();
    void deleteText ();

private:
    void createMenu();
    void createNetworkMenuEntry (ParseUrl &Url);
    void createOtherWidgets();
    void getNetworkProfiles();
    void setTabOrder();
    void setText();
    void connectSignals();
    void deleteFile (const int element);
    
    QToolBar *toolBar;
    QMenu *fileMenu;
    QMenu *networkProfileMenu;
    QAction *openFileAction;
    QAction *openPathAction;
    QAction *addProfileAction;
    QAction *deleteProfileAction;
    QAction *defaultProfileAction;
    QAction *renameAction;
    QAction *deleteAction;
    QAction *settingsAction;
    QAction *cancelAction;
    QAction *aboutAction;

    QString destinationDir {""};
    QPdfDocument *pdfDocument = nullptr;
    QCompleter *completer = nullptr;

    bool Modified(QLineEdit *LineEdit, const std::string Default);
    const std::string defaultTextHost {""}; //{leHost->text().toStdString()};
    const std::string defaultTextDirectory {""}; //{leDirectory->text().toStdString()};
    
    PdfFileList *pdfFileList;
};

QT_END_NAMESPACE

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