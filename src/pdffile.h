#ifndef PDFFILE_H
#define PDFFILE_H

#include <thread>
#include <filesystem>
#include <fstream>
#include <string>
#include <regex>
#include <thread>

// ImageMagick
#include <Magick++.h>

// Tesseract api
#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>
#include <leptonica/allheaders.h>

// libssh
#include <libssh/sftp.h>

// poppler
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-image.h>

//Qt 6.x
#include <QObject>
#include <QCoreApplication>
#include <QMetaMethod>
#include <QMessageBox>
#include <QMainWindow>
#include <QIODevice>
#include <QSaveFile>
#include <QBuffer>

// TessPDFRenderer

// local
#include "parseurl.h"
#include "ftpconnection.h"
#include "settings.h"
#include "scan2ocr.h"

// Version 0.0.3 
class PdfFile : public QObject {
    Q_OBJECT

public:
    PdfFile(ParseUrl Url, QObject *parent = nullptr);

    void initialize();
    bool removeEmptyPages();
    bool transcodeFile();
    bool ocrFile();

    //bool renameFile(const std::string &newName);
    bool removeFile();
    bool saveToFile (const std::string fileName);
    std::string FileName() { return m_possibleFileName; };
    std::shared_ptr<QIODevice> returnFileContent();

    const int getStatusIncrement () { 
        const int c_statusIncrement = 5;
        return  c_statusIncrement;
    };

signals:
    // New status value
    void statusChange();
    void finished();

private:
    ParseUrl m_Url;
    QObject m_parent;
    Settings settings;
    FtpConnection ftpConnection {m_Url};

    std::shared_ptr<QBuffer> pdfBuffer = std::make_shared<QBuffer>();

    int m_Status {0};
    const std::string m_FileName {m_Url.RawFilename()};
    const std::string tempFileName {settings.TmpDir() + m_Url.Filename()};

    std::vector<Magick::Image> m_PdfImgList;
    bool readFile (ParseUrl m_Url);

    std::string m_possibleFileName {""};
    std::string getFileName();
};

class Directory : public QObject {
    Q_OBJECT

public:
    Directory(ParseUrl Url, QObject *parent = nullptr, bool isRecursive = true);
    void initialize();

signals:
    void foundNewFile(std::shared_ptr<ParseUrl> ptr_Url);

private:
    ParseUrl m_Url;
    QObject *p_cfMain {nullptr};
    bool m_isRecursive {false};
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