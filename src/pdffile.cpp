#include "pdffile.h"
#include "scan2ocr.h"

// From https://github.com/bshoshany/thread-pool:
#include "BS_thread_pool.hpp"

#include <filesystem>
#include <fstream>
#include <regex>

/**
 * PdfFile constructor that initializes the object with the given URL, parent QObject, and document profile index.
 *
 * @param Url the ParseUrl object representing the URL
 * @param parent the parent QObject
 * @param documentProfileIndex the index of the document profile
 *
 * @return None
 *
 * @throws None
 */
PdfFile::PdfFile(ParseUrl Url, QObject *parent, int documentProfileIndex) : QObject(parent), m_Url(Url), m_parent(parent), m_documentProfileIndex(documentProfileIndex) {
    if (parent != nullptr) {
        #ifdef DEBUG
            std::cout << "Connecting PdfFile::statusChange and finished (" << this << ") to parent (" << parent << ")" << std::endl;
        #endif

        QObject::connect(this, SIGNAL(statusChange()), parent, SLOT(statusUpdate()), Qt::DirectConnection);
        QObject::connect(this, SIGNAL(finished()), parent, SLOT(filesProcessed()), Qt::DirectConnection);
    }

    // Set documentProfile
    Settings settings;
    documentProfile.isColored = settings.DocumentProfile(m_documentProfileIndex)->isColored;
    documentProfile.thresholdValue = settings.DocumentProfile(m_documentProfileIndex)->thresholdValue;
    documentProfile.resolution = settings.DocumentProfile(m_documentProfileIndex)->resolution;
    documentProfile.language = settings.DocumentProfile(m_documentProfileIndex)->language;
}

/**
 * Initializes the PdfFile object by reading data from a local pdf file or from a remote server.
 * Has to be called after the constructor to have a valid object while emitting signals.
 * @throws None
 */
void PdfFile::initialize() {
    if (m_Url.Scheme() == "file") {
        std::string filename = m_Url.Directory() + "/" + m_Url.Filename();
        readData(readFile(&filename));
    }
    else {
        std::unique_ptr<std::string> remoteFilePtr = ftpConnection.getFilePtr();
        if (remoteFilePtr != nullptr) {
            readData(remoteFilePtr.get());
        }
    }
}

/**
 * Reads the content of a file into a string.
 *
 * @param filename The name of the file to read.
 *
 * @return A pointer to the string containing the file data.
 *
 * @throws None
 */
const std::string *PdfFile::readFile(std::string *filename) {

    std::ifstream file(*filename, std::ios::in | std::ios::binary);
    const static std::string pdfData = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    file.close();
    return &pdfData;
}

/**
 * Reads the content of a PDF file and extracts image streams.
 *
 * @param pdfData A pointer to a string containing the PDF file data.
 *
 * @throws None
*/
void PdfFile::readData(const std::string *pdfData) {

    // Check if it is a pdf file
    const std::string pdfHeader {"%PDF-1."};

    if (pdfData->find(pdfHeader) == std::string::npos) {
        std::cout << "This is not a pdf file!" << std::endl;
        return;
    }

    // Extract all dictionary objects
    const std::regex patternDict("<<([^>]+)>>");
    std::vector<size_t> dictObjectPositions;

    std::sregex_iterator next(pdfData->begin(), pdfData->end(), patternDict);
    std::sregex_iterator end;

    while (next != end) {
        dictObjectPositions.push_back(next->position());
        next++;
    }

    // Extract jpg image stream positions
    constexpr int newLineCharacters = 2;
    constexpr int startCharacters = 10;
    const std::regex patternImageStream("/Filter\\s/DCTDecode");
    std::vector<size_t> imageStreamPositions;

    next = std::sregex_iterator(pdfData->begin(), pdfData->end(), patternImageStream);
    end = std::sregex_iterator();

    while (next != end) {
        imageStreamPositions.push_back(next->position());
        next++;
    }

    startPDF();

    // Extract every image stream
    NumberOfPages = imageStreamPositions.size();

    BS::thread_pool threadPool;
    #define MULTITHREAD

    for (int i = 0; i < NumberOfPages; i++) {
    #ifdef MULTITHREAD
        threadPool.detach_task(
            [&]
            { 
    #endif
                const size_t startPos = imageStreamPositions[i];
                constexpr size_t newLineCharacters = 2;
                const size_t endPos = pdfData->find("endstream", startPos);

                std::string *jpgImage = new std::string(pdfData->substr(startPos, endPos - startPos - newLineCharacters));

                // Remove the pdf image declarations
                const std::regex patternStart(">>(\r\n)stream");
                std::smatch match;
                std::regex_search(*jpgImage, match, patternStart);

                if (match.size() > 1) {
                    *jpgImage = jpgImage->substr(match.position(1) + startCharacters);    
                    processImage(jpgImage, i);
                }
                delete jpgImage;
                jpgImage = nullptr;
            
    #ifdef MULTITHREAD
            }
        );
        threadPool.wait(); 
    #endif
    }

    endPDF();
    emit finished();
}

/**
 * Initializes the PDF file for OCR processing.
 *
 * This function sets up the OCR engine with the appropriate language and page segmentation mode.
 * It also creates a TessPDFRenderer object with the specified output base, Tesseract data path,
 * and text-only flag. Finally, it begins the document with the specified title.
 *
 * @return void
 *
 * @throws None
 */
void PdfFile::startPDF() {
    std::string language;
    switch (documentProfile.language) {
        case Settings::Language::deu:
            language = "deu";
            break;
        case Settings::Language::eng:
            language = "eng";
            break;
        default:
            // Handle unknown language
            break;
    }
    ocr.Init(NULL, language.c_str());
    ocr.SetPageSegMode(tesseract::PageSegMode::PSM_AUTO);

    const std::string outputBaseStr {tempFileName.substr(0, tempFileName.length() - 4)};
    const char  *outputBase = outputBaseStr.c_str();
    const char *tesseractDataPath {ocr.GetDatapath()};
    const bool textonly {false};    
    renderer = std::make_unique<tesseract::TessPDFRenderer>(outputBase, tesseractDataPath, textonly);
    
    const char *documentTitle = m_Url.Filename().c_str();
    renderer->BeginDocument(documentTitle);
}

/**
 * Processes an image (which will be one page of a PDF file) by reading it from a string.
 *
 * @param imageStringData A pointer to a string containing the image data.
 * @param page The page number of the image.
 *
 * @throws None
 */
void PdfFile::processImage (const std::string *imageStringData, int page) {
    
    // First create l_uint8 from std::string
    const l_uint8 *l_uint8Ptr = (const unsigned char *) imageStringData->c_str();
    size_t length {imageStringData->size()};
    
    Pix *pix {pixReadMem(l_uint8Ptr, length)};
    myProgress = timeConstants::MEMORY;
    emit statusChange();

    if (!isEmptyPage(pix)) {
            transcode(pix);
            ocrPage(pix, page);

        if (page == 0) {
            getFileName();
        }   
    }

    pixDestroy(&pix);
}

/**
 * Ends the PDF document by calling the EndDocument method of the renderer object and
 * the End method of the OCR object.
 *
 * @throws None
 */
void PdfFile::endPDF() {
    renderer->EndDocument();
    ocr.End();
}

/**
 * Check if the page represented by the Pix object is considered an empty image based on the average pixel value.
 *
 * @param pix The Pix object representing the image page to be checked.
 *
 * @return True if the image is considered empty 
 * (i.e., the average pixel value is below the threshold set in the Settings object),
 *  false otherwise.
 *
 * @throws None
 */
bool PdfFile::isEmptyPage(Pix *pix) {
    if (!pix) return false;

    // Check if resolution is higher than documentProfile.resolution
    int resolution {documentProfile.resolution};
    int xRes = pixGetXRes(pix);
    int yRes = pixGetYRes(pix);

    if (xRes > resolution || yRes > resolution) {
        pix = pixScale(pix, resolution, resolution);
    }

    /* 
    pixGetPixelAverage:
    * \param[in]    pixs     8 or 32 bpp, or colormapped
    * \param[in]    pixm     [optional] 1 bpp mask over which average is
    *                        to be taken; use all pixels if null
    * \param[in]    x, y     UL corner of pixm relative to the UL corner of pixs;
    *                        can be < 0
    * \param[in]    factor   subsampling factor; >= 1
    * \param[out]   pval     average pixel value
    * */
    PIX *pixm {NULL};
    l_int32 x {0}, y {0};
    l_int32 factor {2};
    std::shared_ptr<l_uint32>imageAverage = std::make_shared<l_uint32>();

    pixGetPixelAverage(pix, pixm, x, y, factor, imageAverage.get());
    double mean_val = static_cast<double>(*imageAverage) / static_cast<double>(std::numeric_limits<l_uint32>::max());
    return (mean_val > settings.thresholdValue()); 
}

/**
 * Transcodes the given Pix object to 1 bpp if settings.isColored is false.
 * Thus the image will be a TIFF G4 encoded object in the final PDF document.
 *
 * @param pix The Pix object to be transcoded.
 *
 * @throws None
 */
void PdfFile::transcode(Pix *&pix) {

        if (!documentProfile.isColored) {
            pix = pixCleanImage(pix, 5, 0, 1, 0);
        }
        myProgress = timeConstants::TRANSCODE;
        emit statusChange();
}

/**
 * Sets the image for OCR processing, recognizes the text, and adds the image to the renderer. 
 *
 * @param pix Pointer to the Pix object representing the image.
 * @param page The page number to be processed.
 *
 * @return None
 *
 * @throws None
 * 
 * 
 */
void PdfFile::ocrPage(Pix *pix, int page) {
    ocr.SetImage(pix);

    tesseract::ETEXT_DESC monitor;
    bool failed {true};

    std::thread recognize_thread(std::bind(&PdfFile::ocrProcess, this, &ocr, &monitor));
    std::thread monitor_thread(std::bind(&PdfFile::monitorProgress, this, &monitor, page)); 
    recognize_thread.join();
    monitor_thread.join();

    renderer->AddImage(&ocr);

    myProgress = timeConstants::OCR;
    emit statusChange();
}

void PdfFile::ocrProcess(tesseract::TessBaseAPI *ocr, tesseract::ETEXT_DESC *monitor) {
    bool failed = ocr->Recognize(monitor) < 0;
}

void PdfFile::monitorProgress(tesseract::ETEXT_DESC *monitor, int page) {
    // The starting Percentage before the ocr has started
    constexpr int minimalPercentage = timeConstants::TRANSCODE;   
    // The fraction of 100% after all ocr is done
    constexpr float maxOCRProgress = timeConstants::OCR / 100.0f;
    const float pageFraction = static_cast<float>(page + 1) / static_cast<float>(NumberOfPages);
    const float minimalProgress = static_cast<float>(minimalPercentage) * pageFraction;

    int monitorProgress {0};
    while (1) {
        if (monitorProgress != monitor->progress) {
            monitorProgress = monitor->progress;
            const int ocrProgress = minimalProgress + static_cast<float>(monitorProgress) * maxOCRProgress * pageFraction;

            // Emit the total progress of this file
            if (myProgress < ocrProgress) {
                myProgress = ocrProgress;
                emit statusChange();
            }
        }
        if (monitorProgress >= 100 || monitorProgress < 0) break;
    }

}
/**
 * Retrieves the file name for the PDF file based on the text content of the file.
 *
 * This function iterates through the text elements of the PDF file using the Tesseract OCR engine,
 * calculates the font size of each text element, and stores the text and font size in a vector.
 * The text elements are then sorted in descending order based on the font size.
 * The function searches for a date in the text content and rearranges it to the format "yyyy mm".
 * It also checks for the presence of an invoice string in the text content.
 * If no date or file descriptor is found, the current date and time are used as the file name.
 * If a date is found, it is added to the file descriptor.
 * If an invoice string is found, it is added to the file name.
 * The resulting file name is emitted through the `statusChange` signal.
 *
 * @return void
 *
 * @throws None
 */
void PdfFile::getFileName() {

    // Check for the largest line of text which does not end with a "." character and has more than 2 characters,
    // which hopefully will be in the majority of cases some descriptive filename
    double font_size {0};
    double largestFontSize {0.0};
    std::string lineWithLargestFontSize {""};

    tesseract::ResultIterator *resultIterator = ocr.GetIterator();
    // Loop through the iterator and get results at word level, calculate bounding box height and fontsize
    tesseract::PageIteratorLevel level = tesseract::RIL_TEXTLINE;
    resultIterator->Begin();

    struct textElement {
        std::string text;
        int font_size;
    };
    std::vector <textElement> textVector;

    while (!resultIterator->Empty(level)) {
       // Get Bounding box dimensions
        int left, top, right, bottom;
        resultIterator->BoundingBox(level, &left, &top, &right, &bottom);
        bool is_bold,is_italic, is_underlined, is_monospace, is_serif, is_smallcaps;
        int pointsize, font_id;
        std::string text = resultIterator->GetUTF8Text(level);
        resultIterator->WordFontAttributes(&is_bold, &is_italic, &is_underlined, &is_monospace, &is_serif, &is_smallcaps, &pointsize, &font_id);
    
        textVector.emplace_back(textElement{text, pointsize});
        resultIterator->Next(level);
    }

    // Sort text in descending order based on fontsize
    auto cmp = [](const textElement& a, const textElement& b) { return a.font_size > b.font_size; };
    std::sort(textVector.begin(), textVector.end(), cmp);

    std::string filedescriptor, datestring, invoicestring;
    bool isInvoice {false};

    // Get translation for invoice if other language than english
    const std::string invoiceString = QObject::tr("Invoice").toStdString();

    // Check for the first occuring date in the format dd.mm.yyyy
    // which can be the date of the letter or invoice
    std::regex pattern("(0[1-9]|[1-2][0-9]|3[0-1])\\.(0[1-9]|1[0-2])\\.(19[0-9]{2}|20[0-9]{2}|2100)");

    // Search for a date in the text
    std::smatch match;

    for (const auto& text_item : textVector) {
        // Take the first useful text (which will be the element with the largest fontsize 
        // and has more than a coule of characters) as a possible file descriptor

        if (filedescriptor.length() == 0 && text_item.text.length() > 3) {
            // Get the first word
            filedescriptor = text_item.text.substr(0, text_item.text.find_first_of(" "));
        }

        if (datestring.length() == 0 && std::regex_search(text_item.text, match, pattern)) {
            // Rearrange the date to the format "yyyy mm"
            // to have the file names sorted according to year than to month
            datestring = match[3].str() + " " + match[2].str() + " ";
        }

        // Check if the text item contains the invoice string
        if (text_item.text.find(invoiceString) != std::string::npos) {
            isInvoice = true;
        }
    }

    if (filedescriptor == "" && datestring == ""){
        // If no date has been found and no useful file descriptor yet exists, then use the current date and time
        m_possibleFileName = getUniqueFileName();
    }
    else if (datestring == "") {
        // Add the current date to the file descriptor if no date has been found
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y %m ");
        m_possibleFileName = ss.str() + filedescriptor;
    }
    else {
        m_possibleFileName = datestring + " " + filedescriptor;
    }

    if (isInvoice) {
        m_possibleFileName += " - " + invoiceString;
    }

    m_possibleFileName += ".pdf";
}

/**
 * Removes the file associated with the PdfFile object.
 *
 * @return true if the file was successfully removed, false otherwise
 *
 * @throws None
 */
bool PdfFile::removeFile() {
    if (m_Url.Scheme() == "file") {
        #ifdef DEBUG
            std::cout << "PdfFile::removeFile: removing file:  " << m_Url.FileDir() << std::endl;
        #else
            if (std::remove(m_Url.FileDir().c_str()) != 0) {
                QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error deleting file ")) + QString::fromStdString(m_Url.FileDir()));
                return false;
            }    
        #endif
    }
    else {
        FtpConnection ftpConnection (m_Url);
        #ifdef DEBUG
            std::cout << "PdfFile::removeFile: removing file: " << m_Url.Url() << std::endl;
        #else
            if (!ftpConnection.deleteFile()) {
                QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error deleting file ")) + QString::fromStdString(m_Url.FileDir()));
                return false;
            }
        #endif
    }
    std::remove(tempFileName.c_str());
    return true;
}

/**
 * Renames the PdfFile to the specified fileName if it does not already exist. Handle potential errors.
 *
 * @param fileName The new file name to rename to.
 *
 * @return true if the file was successfully renamed, false otherwise.
 *
 * @throws std::filesystem::filesystem_error if there are errors during file copying or removal.
 */
bool PdfFile::renameToFileName(const std::string fileName) {
    std::filesystem::copy_options options = std::filesystem::copy_options::none;
    if (std::filesystem::exists(fileName)) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, tr("Warning"), QString(tr("File already exists, overwrite?")), QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return false;
        } 
        else {
            // It should be overwritten
            options = std::filesystem::copy_options::overwrite_existing;
        }
    } 

    // Because the destination file can be on another filesystem we cannot use std::filesystem::rename
    try {
        std::filesystem::copy(tempFileName.c_str(), fileName.c_str(), options);    
    }
    catch (const std::filesystem::filesystem_error& ex) {
        std::cerr << "Error copying temp file to " << fileName << " because: " << ex.what() << std::endl;
        return false;
    }
    if (!std::filesystem::remove(tempFileName.c_str())) {
        std::cerr << "Error removing temp file." << std::endl;
        return false;
    };

    return std::filesystem::exists(fileName);
}

/**
 * Returns the file content as a shared pointer.
 * @throws None
 */
std::shared_ptr<QIODevice> PdfFile::returnFileContent() {
    std::shared_ptr<QIODevice> pdfBuffer = std::make_shared<QFile>(tempFileName.c_str());
    if (pdfBuffer->open(QIODevice::ReadWrite)) {
        QByteArray fileData = pdfBuffer->readAll();
        pdfBuffer->write(fileData);
    }
    return std::move(pdfBuffer);
}

/**
 * Constructs a new Directory object with the given ParseUrl, parent QObject,
 * recursion flag, and document profile index.
 *
 * @param Url the ParseUrl object representing the directory URL
 * @param parent the parent QObject
 * @param isRecursive a flag indicating whether to recurse through subdirectories
 * @param documentProfileIndex the index of the document profile
 *
 * @throws None
 */
Directory::Directory (ParseUrl Url, QObject *parent, bool isRecursive, int documentProfileIndex) : 
    QObject (parent), m_Url(Url), p_cfMain (parent), m_isRecursive (isRecursive), m_documentProfileIndex(documentProfileIndex) {

}

/**
 * Initializes the Directory object by scanning a local or remote directory for subdirectories and pdf files.
 * Creates instances of PdfFile for each .pdf file and Directory for each subdirectory.
 * Has to be called after the constructor to have a valid object while emitting signals.
 *
 * @throws None
 */
void Directory::initialize() {
    #ifdef DEBUG 
        std::cout << "Directory(): New Directory: " << m_Url.Url() << " p_cfMain: " << p_cfMain << " m_isRecursive: " << m_isRecursive << std::endl;
    #endif
    
    auto addPdf = [&](const auto& entry) {
        if (entry.path().extension() == ".pdf") {
            ParseUrl new_Url (m_Url);
            new_Url.FileDir(entry.path().string());
            std::shared_ptr<ParseUrl>ptr_Url = std::make_shared<ParseUrl>(new_Url);
            emit foundNewFile(ptr_Url, m_documentProfileIndex);
        }
    };

    if (m_Url.Scheme() == "file") {
        // Screen lokal directory for subdirectories and pdf files
        // Create an instance of PdfFile for each .pdf file and an instance of Directory for each subdirectory
        if (m_isRecursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(m_Url.Directory())) {
                addPdf(entry);
            }
        }   
        else { // No recursion through subdirs
            for (const auto& entry : std::filesystem::directory_iterator(m_Url.Directory())) {
                addPdf(entry);
            }
        }       
    }
    else {
        // Screen remote directory for subdirectories and pdf files and create instances of PdfFile
        FtpConnection ftpConnection (m_Url);
        std::unique_ptr<std::vector<std::string>> remoteDirPtr = ftpConnection.getRemoteDir(m_isRecursive);
        if (remoteDirPtr != nullptr) {
            for (std::string entry: *(remoteDirPtr)) {
                // Construct new Url on m_Url with entry, which is the directory and filename (= FileDir)
                std::shared_ptr<ParseUrl> ptr_newUrl = std::make_shared<ParseUrl> (m_Url);
                ptr_newUrl->FileDir(entry);
                #ifdef DEBUG
                    std::cout << "Directory(), entry: " << entry << std::endl;
                    std::cout << "Directory(), found new file with Url: " << ptr_newUrl->Url() << std::endl;
                    std::cout << "Creating new instance of PdfFile with p_cfMain to " << p_cfMain << std::endl;
                    std::cout << "foundNewFile from Directory()" << std::endl;
                #endif
                emit foundNewFile(ptr_newUrl, m_documentProfileIndex);
            }    
        }
        
    }
}


/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email ( at ) simonboettger . de

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