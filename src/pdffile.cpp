#include "pdffile.h"
PdfFile::PdfFile(ParseUrl Url, QObject *parent) : QObject(parent), m_Url(Url), m_parent(parent) {
    if (parent != nullptr) {
        #ifdef DEBUG
            std::cout << "Connecting PdfFile::statusChange and finished (" << this << ") to parent (" << parent << ")" << std::endl;
        #endif

        QObject::connect(this, SIGNAL(statusChange()), parent, SLOT(statusUpdate()), Qt::DirectConnection);
        QObject::connect(this, SIGNAL(finished()), parent, SLOT(filesProcessed()), Qt::DirectConnection);
    }
    #ifdef DEBUG
        else {
            std::cout << "PdfFile initialized with parent nullptr " << std::endl;
        }
    #endif
}

void PdfFile::initialize(){
    #ifdef DEBUG
        std::cout << "PdfFile::initialize on: " << m_Url.Url() << std::endl;
    #endif 
    
    m_Status++;
    emit statusChange();
    bool retVal = readFile(m_Url);
    
    m_Status++;
    emit statusChange();
    retVal =removeEmptyPages();
    
    m_Status++;
    emit statusChange();
    retVal = transcodeFile();
    
    m_Status++;
    emit statusChange();
    retVal = ocrFile();

    m_Status++;
    emit statusChange();
    m_possibleFileName = getFileName();

    emit finished();
    #ifdef DEBUG
        std::cout << "PdfFile::finished from " << this << std::endl;
    #endif
}

bool PdfFile::readFile(ParseUrl Url) {
    // Read local PdfFile into Memory
    Magick::ReadOptions options;
    int resolution {settings.resolution()};
    options.density(Magick::Geometry(resolution, resolution));
    
    if (Url.Scheme() == "file") {
        Magick::readImages(&m_PdfImgList, m_Url.Directory() + "/" + m_Url.Filename(), options);
    }
    else {
        std::unique_ptr<Magick::Blob> remoteFilePtr = ftpConnection.getFile();
        if (remoteFilePtr != nullptr) {
            Magick::readImages(&m_PdfImgList, *remoteFilePtr, options);
        }
    }
    
    return true;
}

bool PdfFile::removeEmptyPages() {
    m_PdfImgList.erase(
        std::remove_if(
            m_PdfImgList.begin(),
            m_PdfImgList.end(),
            [this](Magick::Image& image) {
                Magick::ImageStatistics statistics = image.statistics();
                double mean_val = statistics.channel(MagickCore::PixelChannel()).mean() / (1 << 16);
                return mean_val > settings.thresholdValue();
            }
        ),
        m_PdfImgList.end()
    );
    return true;
}

bool PdfFile::transcodeFile() {
    const bool isColored {settings.isColored()};
    for (auto &image : m_PdfImgList)
    {
        // Turn into black and white image if isColored == false
        if (!isColored) {
            image.autoThreshold(MagickCore::AutoThresholdMethod::OTSUThresholdMethod);
            image.threshold(settings.thresholdValue());

            // Change encoding to Tiff G4
            image.magick("TIFF");
            image.depth(8);
            image.compressType(Magick::CompressionType::Group4Compression);
        }
        else {
            // If isColored == true
            image.depth(8);
            image.compressType(Magick::CompressionType::JPEGCompression);
        }
        
    }
    return true;
}

bool PdfFile::ocrFile() {
    #ifdef DEBUG
        std::cout << "PdfFile::ocrFile() on: " << m_Url.Url() << std::endl;
    #endif

    tesseract::TessBaseAPI ocr;
    ocr.Init(NULL, settings.language().c_str(), tesseract::OEM_LSTM_ONLY);

    // Set page segmentation mode (default option is best): ocr.SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_BLOCK);
    constexpr bool textOnly {false};
    const std::string outputBase {"/tmp/" + m_Url.RawFilename()};
    
    tesseract::TessPDFRenderer *renderer = new tesseract::TessPDFRenderer(outputBase.c_str(), ocr.GetDatapath(), textOnly);
    //PdfStream *pdfStreamRenderer = new PdfStream(outputBase.c_str(), ocr.GetDatapath(), textOnly);

    const std::string inputFileName {"/tmp/" + m_Url.RawFilename()+ ".tiff"};
    constexpr char *retryConfig {nullptr};
    constexpr int timeout_millisec {0};
    constexpr bool adjoin {true};

    // Quite nasty hack with tempfiles because tessPDFRenderer does not take any memory objects or writes a pdf stream to memory
    // Magick::writeImages(m_PdfImgList.begin(), m_PdfImgList.end(), inputFileName, adjoin);
    // bool retVal = ocr.ProcessPages (inputFileName.c_str(), retryConfig, timeout_millisec, renderer);
    
    // Make ocr on pix object from TIFF 
    Pix *pix {nullptr};
    Magick::Blob rawData;
    Magick::writeImages(m_PdfImgList.begin(), m_PdfImgList.end(), &rawData, adjoin);

    const l_uint8 *imgData = reinterpret_cast<const l_uint8*>(rawData.data());
    size_t size = rawData.length();
    size_t offset = 0;
    int page {0};
    
    renderer->BeginDocument(m_FileName.c_str());
    
    // only works for tiff files, implement sth. for jpeg if isColored == true
    if (settings.isColored()) {
        for (;; ++page) {
            pix = pixReadMemFromMultipageTiff(imgData, size, &offset);
            if (pix == nullptr) {
                break;
            }
            if (offset || page > 0) {
            // Only print page number for multipage TIFF file.
            std::cout << "Page %d\n" << page + 1 << std::endl;
            }
            auto page_string = std::to_string(page);
            
            ocr.SetImage(pix);
            pixDestroy(&pix);
            bool failed {false};
            if (!std::unique_ptr<const tesseract::PageIterator>(ocr.AnalyseLayout())) {;
                failed = true;
            }
            failed = ocr.Recognize(nullptr) < 0;

            if (renderer && !failed) {
                failed = !renderer->AddImage(&ocr);
            }
            if (failed) {
                QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error writing pdf stream")) + QString::fromStdString(tempFileName));
                return false;
            }

            if (!offset) {
            break;
            }
        }
    }
    else {
        pix = pixReadMem(imgData, size);
        ocr.SetImage(pix);
        pixDestroy(&pix);

        bool failed {false};
        if (!std::unique_ptr<const tesseract::PageIterator>(ocr.AnalyseLayout())) {;
            failed = true;
        }
        failed = ocr.Recognize(nullptr) < 0;

        if (renderer && !failed) {
            failed = !renderer->AddImage(&ocr);
        }
        if (failed) {
            QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error writing pdf stream")) + QString::fromStdString(tempFileName));
            return false;
        }
    }
    renderer->EndDocument();

    //read tempfile into buffer
    pdfBuffer->open(QIODevice::ReadWrite);
    QFile file(tempFileName.c_str());
    if(!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open temporary pdf file for reading: " << tempFileName.c_str();
        return false;
    }
    QByteArray data = file.readAll();
    pdfBuffer->write(data);

    // Now cleanup the mess we left on /tmp
    file.close();
    pdfBuffer->close();
    std::remove(tempFileName.c_str());
 
    ocr.End();
    pixDestroy(&pix);
    delete renderer;
    return true;
}

std::string PdfFile::getFileName() {
    #ifdef DEBUG
        std::cout << "PdfFile::getFileName() on: " << m_Url.Url() << std::endl;
    #endif
    pdfBuffer->open(QIODevice::ReadOnly);
    std::unique_ptr<poppler::document> pdfDoc(poppler::document::load_from_raw_data (pdfBuffer->data(), pdfBuffer->size()));
    pdfBuffer->close(); 
    if (!pdfDoc) {
        QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error reading file content")));
        return nullptr;
    }

    // Get the first page of the PDF
    std::unique_ptr<poppler::page> firstPage(pdfDoc->create_page(0));
    if (!firstPage) {
        QMessageBox::critical(nullptr, tr("Error"), QString(tr("Error reading first page")));
        return nullptr;
    }

    // Check for the largest line of text which does not end with a "." character and has more than 2 characters,
    // which hopefully will be in the majority of cases some descriptive filename
    double font_size {0};
    double largestFontSize {0.0};
    std::string lineWithLargestFontSize {""};
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
    std::string text {firstPage->text().to_latin1().c_str()};
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
    
    #ifdef DEBUG
        std::cout << "PdfFile::getFileName(), possibleFileName: " << possibleFileName << std::endl;
    #endif

    return possibleFileName;
}

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
    return true;
}

bool PdfFile::saveToFile(const std::string fileName) {
    #ifdef DEBUG
        std::cout << "PdfFile::saveToFile on: " << fileName << std::endl;
    #endif
    
    if (std::filesystem::exists(fileName)) {
        QMessageBox msgBox;
        msgBox.setText(QString::fromStdString(fileName));
        msgBox.setInformativeText(tr("File already exists. Overwrite?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        if (ret == QMessageBox::No) {
            return false;
        }
    }
    
    pdfBuffer->open(QIODevice::ReadOnly);
    QByteArray data = pdfBuffer->readAll();
    pdfBuffer->close();
    std::ofstream outputFile(fileName, std::ios::binary);
    if (!outputFile) {
        qWarning() << "Failed to open file for writing: " << fileName.c_str();
        return false;
    }
    outputFile.write(data.data(), data.size());
    outputFile.close();
    return true;
}

std::shared_ptr<QIODevice> PdfFile::returnFileContent() {
    #ifdef DEBUG
        std::cout << "PdfFile::returnFileContent on: " << m_Url.Url() << std::endl;
    #endif
    pdfBuffer->open(QIODevice::ReadOnly);
    return pdfBuffer;
}

Directory::Directory (ParseUrl Url, QObject *parent, bool isRecursive) : QObject (parent), m_Url(Url), p_cfMain (parent), m_isRecursive (isRecursive) {

}

void Directory::initialize() {
    #ifdef DEBUG 
        std::cout << "Directory(): New Directory: " << m_Url.Url() << " p_cfMain: " << p_cfMain << " m_isRecursive: " << m_isRecursive << std::endl;
    #endif
    
    auto addPdf = [&](const auto& entry) {
        if (entry.path().extension() == ".pdf") {
            ParseUrl new_Url (m_Url);
            new_Url.FileDir(entry.path().string());
            std::shared_ptr<ParseUrl>ptr_Url = std::make_shared<ParseUrl>(new_Url);
            emit foundNewFile(ptr_Url);
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
                emit foundNewFile(ptr_newUrl);
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