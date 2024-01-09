#include "pdffile.h"

// ************************************************************************************************
//                                  PdfFileList class
// ************************************************************************************************
//
// Class for managing pdf files. It has a list of pointers to instances of Class PdfFile
// addFiles adds files either locally ore if they are on a sftp server it first transfers them
// to a lokal tempfile.
// removeFile takes care of removing all tempfiles and the original pdf file either locally or on
// the sftp server
// PdfFile represents one pdf file. This class manages the processing of each pdf file
//
// ************************************************************************************************

/*
Data flow:
source:   local file                        sftp file
            |                                   |   
            |                                local copy
            |                                   |
            ------- read copy into memory -------
            |                                   |    
          remove empty page, transcode to tiff g4
            |                                   |    
            --- write temp file for tesseract----
            |                                   |
            ------------ add ocr layer ---------
            |                                   |   
            ------ guess possible filename -----
            |                                   |   
             read preview of pdf from temp file 
            |                                   |
            -- user chooses definite filename --
            |                                   |
destination: move temp file to definite filename
            |                                   |
cleanup:    ----------- remove temp file --------
            |                                   |
            |                           remove local copy
            |                                   |
        remove local file                   remove sftp file

local file and sftp file are stored by using a url (file:/// or sftp://)
local copy is equal to local file if the source pdf is a local file. 
Its format is in the form /directory/filename
*/

PdfFileList::~PdfFileList() {
    for (auto pdfFile : pdfFiles) {
        if (pdfFile != nullptr) {
            delete pdfFile;
        }
    }
    pdfFiles.clear();
}

bool PdfFileList::addFiles(ParseUrl &Url) {
    int numThreads {0};
    std::vector<std::thread> threads;

    if (Url.Scheme() == "file") {
        // Get lokal files
        if (Url.Filename() != "") {
            // Add single local file

            // each new file requires a number of processing steps equal to c_statusIncrement
            // if one step of one pdf file is ahead, increment status by 1
            // status is updated in PdfFile using m_Status which is passed by ref
            maxStatus += c_statusIncrement;
            emit newFileAdded();

            PdfFile* pdfFile = new PdfFile(Url, Url.FileDir(), m_Status);
            pdfFiles.push_back(pdfFile);
            emit newFileComplete(Url.Filename());
            
            emit finishedProcessing();
            return true;
        }
        else {
            // Get whole directories including subdirectories recursesively
            for (const auto& entry : std::filesystem::recursive_directory_iterator(Url.Directory())) {
                if (entry.path().extension() == ".pdf") {
                    
                    maxStatus += c_statusIncrement;
                    emit newFileAdded();
                    QCoreApplication::processEvents();

                    ParseUrl newUrl ("file://" + entry.path().string());

                    auto lambdaThread = [=]()-> void{
                        PdfFile* pdfFile = new PdfFile (newUrl, newUrl.FileDir(), m_Status);
                        pdfFiles.push_back(pdfFile);

                        emit newFileComplete(entry.path().filename().string());
                    };
                    threads.push_back(std::thread(lambdaThread));
                }
            }
        }
    }
    else {
        // Scheme will be sftp, so we call a remote computer
        if (Url.Filename() != "") {
            // Only a single remote File
            maxStatus += c_statusIncrement;
            emit newFileAdded();

            FtpConnection ftpConnection (Url);

            std::string filename = ftpConnection.getFile(Url.Filename(), Url.Directory());
            PdfFile* pdfFile = new PdfFile(Url, filename, m_Status);
            pdfFiles.push_back(pdfFile);
            emit newFileComplete(Url.Filename());

            emit finishedProcessing();
            return true;
        }
        else {
            // Remote directory(ies)
             // Loop through all pdf files in the given RemoteDirectory and all its subdirectories
            FtpConnection ftpConnection(Url);

            const std::string directory = Url.Directory();
            std::vector<sftp_attributes> sftpAttributes = ftpConnection.getRemoteDir(directory);

            // Check if we have at least one directory returned, otherwise return failure
            if (sftpAttributes.empty()) {
                return false;
            }

            const std::string fileDirectory = directory;
            for(auto& attr : sftpAttributes) {
                // Make sure we don't step into the current directory again or the parent directory,
                // but we also checked all subdirectories
                if (attr->type == SSH_FILEXFER_TYPE_DIRECTORY && std::strcmp(attr->name, ".") != 0 && std::strcmp(attr->name, "..") != 0) {
                    // Found subdirectory, loop through it using recursion
                    const std::string subdirectory = attr->name;
                    ParseUrl newUrl (Url);
                    newUrl.Directory(newUrl.Directory() + attr->name + "/");
                    addFiles (newUrl);
                }
                else if (attr->type == SSH_FILEXFER_TYPE_REGULAR) {
                    // Found a file, if pdf file add it to list
                    std::string filename = attr->name;
                    
                    // get file from sftp server if pdf
                    if (filename.size() > 3 && filename.substr(filename.length() - 3) == "pdf"){
                        auto lambdaThread = [=]()-> void{
                            maxStatus += c_statusIncrement;
                            emit newFileAdded();

                            // We are in a new thread, get a new ftpConnection                            
                            FtpConnection ftpConThread(Url);

                            const std::string tmpFileName = ftpConThread.getFile(filename, fileDirectory);
                            ParseUrl newUrl(Url);
                            newUrl.Filename(filename);
                            newUrl.Directory(fileDirectory);
                            PdfFile* pdfFile = new PdfFile(newUrl, tmpFileName, m_Status);
                            pdfFiles.push_back(pdfFile);

                            emit newFileComplete(filename);
                        };

                        threads.push_back(std::thread(lambdaThread));
                    }
                }
            } 
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    emit finishedProcessing();
    return true;
}

bool PdfFileList::removeFile(int Element) {
    // Check pdfFiles[Element]
    if (pdfFiles[Element] == nullptr) {
        return false;
    }

    // First delete sourcefile
    ParseUrl Url = pdfFiles[Element]->getUrl();
    if (Url.Scheme() == "file") {
        std::filesystem::path localFile {Url.FileDir()};
        try {
            std::filesystem::remove(localFile);
        } catch (std::filesystem::filesystem_error& e) {
            QMessageBox::critical(nullptr, tr("Error Deleting file"), QString::fromStdString(e.what()));
            return false;
        }
    }
    else if (Url.Scheme() == "sftp") {

        FtpConnection ftpConnection (Url);
        ftpConnection.deleteFile(Url.FileDir());
    }

    // Delete instance of _pdfFile, remove from pdfFileList
    delete pdfFiles[Element];
    pdfFiles.erase(pdfFiles.begin() + Element);
    return true;
}

// *******************************************************************************************************
//                                          PdfFile class
// *******************************************************************************************************
//
// It takes the url to a local pdf file
// It first removes every empty page
// then it converts the encoding to black and white with tiff g4
// it adds an ocr layer
// and guesses a possible filename based on the largest line of text on the first page

PdfFile::PdfFile(ParseUrl Url, const std::string &localCopy, int &status, QObject *parent) : QObject(parent), m_Url(Url), m_LocalCopy(localCopy) {
    if (ptr_cfMain != nullptr) {
        QObject::connect(this, SIGNAL(statusChange()), ptr_cfMain, SLOT(statusUpdate()), Qt::DirectConnection);
    }
    
    // Read PdfFile into Memory
    Magick::ReadOptions options;
    options.density(Magick::Geometry(600, 600));        // Set Resolution of input image to 600 dpi
    Magick::readImages(&pdfImgList, m_LocalCopy, options);
    
    status++;
    emit statusChange();
    removeEmptyPage();

    status++;
    emit statusChange();
    transcodeToTiff();

    status++;
    emit statusChange();
    ocrPdf();

    status++;
    emit statusChange();
    m_possibleFileName = getPossibleFileName();
}

bool PdfFile::removeEmptyPage() {
    for (auto it = pdfImgList.begin(); it != pdfImgList.end();)
    {
        Magick::Image &image = *it;
        Magick::ImageStatistics Statistics = image.statistics();
        double mean_val = Statistics.channel(MagickCore::PixelChannel()).mean();
        mean_val /= 1 << 16;
        if (mean_val > Threshold)
        {
            it = pdfImgList.erase(it);
        }
        ++it;
    }
    return true;
}

bool PdfFile::transcodeToTiff() {

    for (auto &image : pdfImgList)
    {
        // Turn into black and white image
        image.autoThreshold(MagickCore::AutoThresholdMethod::OTSUThresholdMethod);

        // Change encoding to Tiff G4
        image.magick("TIFF");
        image.compressType(Magick::CompressionType::Group4Compression);
    }
    return true;
}

bool PdfFile::ocrPdf() {
    tesseract::TessBaseAPI ocr;
    ocr.Init(NULL, Language, tesseract::OEM_LSTM_ONLY);

    // Tesseract always adds .pdf to the file name
    // To have the output of tesseract into m_tempFileName remove the .pdf from m_tempFileName
    const std::string &InputOCR = m_tempFileName.substr(0, m_tempFileName.find_last_of("."));
    Magick::writeImages(pdfImgList.begin(), pdfImgList.end(), InputOCR, true);

    // Set page segmentation mode (default option is best): ocr.SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_BLOCK);
    tesseract::TessPDFRenderer *renderer = new tesseract::TessPDFRenderer(InputOCR.c_str(), ocr.GetDatapath());

    // Write OCRPDF to OutputFile
    bool retVal = ocr.ProcessPages(InputOCR.c_str(), NULL, 0,  renderer);
    if (!retVal) {
        QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error writing output pdf ")) + QString::fromStdString(m_tempFileName));
        return false;
    }

    // Remove Inputfile, keep HOCR File which should now be equal to m_tempFileName (Filename + .pdf)
    std::filesystem::remove(InputOCR);

/*     // Andere Möglichkeit über pixObject:
    Alles in einer Schleife, Funktionen zusammenlegen. Fortschritt über Seite 1/n
    #include <leptonica/allheaders.h>
    Magick::Blob blob;
    Pix* pixImage; 

    for (auto& image : pdfImgList) {
        image.write(&blob);
        pixImage = pixReadMem(reinterpret_cast<const l_uint8*>(blob.data()), blob.length());
        ocr.SetImage(pixImage);
        ocr.Recognize(nullptr);
        renderer->AddImage(&ocr);
        pixDestroy(&pixImage);
    }
    renderer->EndDocument(); */

    delete renderer;
    ocr.End();
    return true;
}

std::string PdfFile::getPossibleFileName() {

    std::unique_ptr<poppler::document> pdfDoc(poppler::document::load_from_file (m_tempFileName.c_str()));
    
    if (!pdfDoc) {
        QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error opening ")) + QString::fromStdString(m_tempFileName));
        return "";
    }

    // Get the first page of the PDF
    std::unique_ptr<poppler::page> firstPage(pdfDoc->create_page(0));

    if (!firstPage) {
        QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error getting first page")));
        return "";
    }

    double font_size = 0;
    double largestFontSize = 0.0;
    std::string lineWithLargestFontSize="";

    // Check for the largest line of text which does not end with a "." character and has more than 2 characters,
    // which hopefully will be in the majority of cases some descriptive filename

    for (const auto& text_item : firstPage->text_list(1))   //opt_flag has to be 1 to get the fontsize
    {
        font_size = text_item.get_font_size();
        if (font_size > largestFontSize && text_item.text().length() > 2 && text_item.text().back() != '.'){
            largestFontSize = font_size;
            lineWithLargestFontSize = text_item.text().to_latin1().c_str();
        }
    }
    std::string possibleFileName = lineWithLargestFontSize;

    // Check for the first occuring date in the format dd.mm.yyyy
    // which can be the date of the letter or invoice
    std::string text = firstPage->text().to_latin1().c_str();
    std::regex pattern("(0[1-9]|[1-2][0-9]|3[0-1])\\.(0[1-9]|1[0-2])\\.(19[0-9]{2}|20[0-9]{2}|2100)");

    // Search for a date in the text
    std::smatch match;
    if (std::regex_search(text, match, pattern)) {
        // Rearrange the date to the format "yyyy mm"
        // to have the file names sorted according to year than to month
        std::string rearrangedDate = match[3].str() + " " + match[2].str() + " ";
        possibleFileName.insert (0, rearrangedDate);
    }
    else if (possibleFileName == ""){
        // If no date has been found, then use the current date and time
        possibleFileName = getUniqueFileName();
    }
    else {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y %m ");
        possibleFileName = ss.str() + possibleFileName;
    }

    // Check if the file is an invoice
    // first we have to get the current application language
    const std::string invoiceString {QString(tr("Invoice")).toStdString()};

    if (text.find(invoiceString) != std::string::npos) {
        possibleFileName += " - " + invoiceString;
    }

    possibleFileName += ".pdf";

    return possibleFileName;
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