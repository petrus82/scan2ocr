#pragma once

#include "pdffile.h"
#include "parseurl.h"
#include "settings.h"

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

class MainWindow : public QMainWindow {

Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    //void addProfile();
    //void deleteProfile();
    //void defaultProfile();
    void settings();
    void cancel();
    void about();
    void deleteText ();

private:

    void createMenu();
    //void createNetworkMenuEntry (Scan2ocr::s_networkProfile &netProfile);
    void createOtherWidgets();
    //std::vector<std::unique_ptr<s_networkProfile>> getNetworkProfiles();
    void setTabOrder();
    void setText();
    void connectSignals();
    void deleteFile (const int element);
    
    QWidget centralWidget {this};
    QGridLayout mainLayout {&centralWidget};
    QPdfView pdfView {&centralWidget};
    QToolBar toolBar {this};
    QListWidget lsFiles {&centralWidget};
    QLineEdit leFileName {&centralWidget};
    QPushButton pbRename {&centralWidget};
    QProgressBar pbProgress {&centralWidget};
    QLineEdit leDestinationDir {&centralWidget};
    QLabel lbDestinationDir {&centralWidget};
    QPushButton pbDestinationDir {&centralWidget};

    QMenu *fileMenu {menuBar()->addMenu(tr("F&ile"))};
    QMenu *networkProfileMenu {nullptr};
    QAction openFileAction {tr("&Open File..."), this};
    QAction openPathAction {tr("Open &Directory..."), this};
    QAction addProfileAction {tr("Add &Profile..."), this};
    QAction deleteProfileAction {tr("Delete &Profile..."), this};
    QAction defaultProfileAction {tr("Default &Profile..."), this};
    QAction renameAction {tr("&Rename File"), this};
    QAction deleteAction {tr("De&lete File"), this};
    QAction settingsAction {tr("Se&ttings..."), this};
    QAction cancelAction {tr("&Quit"), this};
    QAction aboutAction {tr("Abou&t"), this};
    std::unique_ptr<QComboBox> cbNetworkProfiles {nullptr};
    std::unique_ptr<QAction> networkProfileAction {nullptr};

    QString destinationDir {""};
    QPdfDocument pdfDocument;
    QCompleter completer;

    bool Modified(QLineEdit *LineEdit, const std::string Default);
    const std::string defaultTextHost {""}; //{leHost->text().toStdString()};
    const std::string defaultTextDirectory {""}; //{leDirectory->text().toStdString()};
    
    PdfFileList *pdfFileList {nullptr};
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