#ifndef PDFFILE_H
#define PDFFILE_H

#include <thread>
#include <filesystem>
#include <fstream>
#include <string>
#include <regex>
#include <thread>

#include <Magick++.h>
#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>
#include <libssh/sftp.h>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <QObject>
#include <QCoreApplication>
#include <QMetaMethod>
#include <QMessageBox>
#include <QMainWindow>

#include "parseurl.h"
#include "ftpconnection.h"
#include "settings.h"
#include "scan2ocr.h"

class PdfFile;

class PdfFileList : public QObject {
    Q_OBJECT

public:
    static PdfFileList& get_instance(){
        static PdfFileList instance;
        return instance;    
    }

    std::unique_ptr<PdfFile>&pdfFile (int Element) {
        return pdfFiles[Element];
    };
    
    int maxFiles() const { return pdfFiles.size(); };
    
    bool addFiles (ParseUrl &Url);
    bool removeFile (int Element);

    int status () const { return m_Status;};
    void status (int status) { m_Status = status;};

    QMainWindow *instance_cfMain () { return p_cfMain; };
    void instance_cfMain (QMainWindow *ptr) { p_cfMain = ptr; };

    int maxStatus {0};

public slots:

signals:
    // After a new file has been found and maxStatus is updated
    void newFileAdded();    
    
    // New file has been processed and
    // pdfFile is pushed onto pdfFileList, ready to load pdf
    void newFileComplete(const std::string Filename);

    // After all files have been processed
    void finishedProcessing();

private:
    // Make PdfFileList Singleton
    PdfFileList() = default;
    ~PdfFileList();
    PdfFileList(const PdfFileList&) = delete;
    PdfFileList& operator=(const PdfFileList&) = delete;

    int m_Status {0};
    std::vector<std::unique_ptr<PdfFile>> pdfFiles;
    static constexpr const int c_statusIncrement = 4;
    QMainWindow *p_cfMain {nullptr};                // Ptr to instance of cfMain to connect statusUpdate SLOT
    
};
extern PdfFileList &pdfFileList;

class PdfFile : public QObject {
    Q_OBJECT
public:
    PdfFile(ParseUrl Url, const std::string &localCopy, int &status, QObject *parent = nullptr);

    ParseUrl getUrl() { return m_Url; };
    
    std::list<Magick::Image> pdfImgList;

    const std::string possibleFileName() { return m_possibleFileName; };
    const std::string tempFileName() { return m_tempFileName; };

    const std::string localCopy () {return m_Url.Filename();};
    void localCopy (std::string Filename) { m_LocalCopy = Filename; };

signals:
    // New status value
    void statusChange();

private:
    bool transcodeToTiff();
    bool removeEmptyPage();
    bool ocrPdf();
    std::string getPossibleFileName ();

    ParseUrl m_Url;
    Settings settings;
    QMainWindow *ptr_cfMain = dynamic_cast<QMainWindow*>(PdfFileList::get_instance().instance_cfMain());
    
    std::string m_possibleFileName;
    std::string m_tempFileName {settings.TmpDir() + getUniqueFileName()};
    std::string m_LocalCopy {""};
  
};

#endif

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