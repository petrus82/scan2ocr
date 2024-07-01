#ifndef PDFFILE_H
#define PDFFILE_H


#include <string>

// Tesseract api
#include <tesseract/baseapi.h>
#include <tesseract/renderer.h>
#include <tesseract/ocrclass.h>
#include <leptonica/allheaders.h>

//Qt 6.x
#include <QObject>
#include <QIODevice>
#include <QBuffer>

// local
#include "parseurl.h"
#include "ftpconnection.h"
#include "settings.h"
#include "scan2ocr.h"

class PdfFile : public QObject {
    Q_OBJECT

public:
    PdfFile(ParseUrl Url, QObject *parent = nullptr, int documentProfileIndex = 0);
    void initialize();
    
    // file handling
    bool removeFile();
    bool renameToFileName (const std::string fileName);
    std::string FileName() { return m_possibleFileName; };
    std::shared_ptr<QIODevice> returnFileContent();
    const char *pdfFileName() { return tempFileName.c_str(); };

    int Progress () const { return myProgress; };

signals:
    // New status value
    void statusChange();
    // All processing done
    void finished();

private:
    // How many times statusChange is emitted for one image page
    // Will be 100 x from monitor->progress and 4 x from other functions
    //static constexpr int cStatusIncrement = 104; 

    enum timeConstants {
        MEMORY = 5,
        TRANSCODE = 15,
        OCR = 80
    };

    // How many times statusChange will be called in total from this class
    int NumberOfPages {0};

    ParseUrl m_Url;
    QObject m_parent;
    Settings settings;
    FtpConnection ftpConnection {m_Url};
    tesseract::TessBaseAPI ocr;
    std::unique_ptr<tesseract::TessPDFRenderer>renderer;
    
    const std::string tempFileName = settings.TmpDir() + m_Url.Filename();
    const std::string *readFile (std::string *pdfFile);
    void readData (const std::string *pdfData);
    void startPDF();
    void endPDF();

    void processImage (const std::string *imageStringData, int page);
    bool isEmptyPage(Pix *pix);
    void transcode (Pix *&pix);
    void ocrPage (Pix *pix, int page);

    void monitorProgress(tesseract::ETEXT_DESC *monitor, int paget);
    void ocrProcess(tesseract::TessBaseAPI *api, tesseract::ETEXT_DESC *monitor);

    int myProgress {0};

    std::string m_possibleFileName {""};
    void getFileName();

    int m_documentProfileIndex;
    Settings::documentProfile documentProfile;
};

class Directory : public QObject {
    Q_OBJECT

public:
    Directory(ParseUrl Url, QObject *parent = nullptr, bool isRecursive = true, int documentProfileIndex = 0);
    void initialize();

signals:
    void foundNewFile(std::shared_ptr<ParseUrl> ptr_Url, int documentProfileIndex);

private:
    ParseUrl m_Url;
    QObject *p_cfMain {nullptr};
    bool m_isRecursive {false};
    int m_documentProfileIndex;
};

#endif

/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email (at) simonboettger . de

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