#include "pdfparse.h"

void PdfParse::parsePDF(std::string *filename) {

    // Read file into string
    std::ifstream file(*filename, std::ios::in | std::ios::binary);
    std::string pdfData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Check if it is a pdf file
    const std::string pdfHeader {"%PDF-1."};

    if (pdfData.find(pdfHeader) == std::string::npos) {
        std::cout << "File is not a pdf file" << std::endl;
        return;
    }

    // Extract all dictionary objects
    const std::regex patternDict("<<([^>]+)>>");
    std::vector<size_t> dictObjectPositions;

    std::sregex_iterator next(pdfData.begin(), pdfData.end(), patternDict);
    std::sregex_iterator end;

    while (next != end) {
        dictObjectPositions.push_back(next->position());
        next++;
    }

    // Extract image stream positions
    constexpr int newLineCharacters = 2;
    constexpr int startCharacters = 10;
    const std::regex patternImageStream("/Filter\\s/DCTDecode");
    std::vector<size_t> imageStreamPositions;

    next = std::sregex_iterator(pdfData.begin(), pdfData.end(), patternImageStream);
    end = std::sregex_iterator();

    while (next != end) {
        imageStreamPositions.push_back(next->position());
        next++;
    }

    // Extract image streams
    for (size_t i = 0; i < imageStreamPositions.size(); i++) {
        size_t startPos = imageStreamPositions[i];
        constexpr size_t newLineCharacters = 2;
        size_t endPos = pdfData.find("endstream", startPos);

        jpgImages.get()->emplace_back(std::make_shared<std::string>(pdfData.substr(startPos, endPos - startPos - newLineCharacters)));

        // Now remove the pdf image declarations
        const std::regex patternStart(">>(\r\n)stream");
        std::smatch match;
        std::regex_search(jpgImages.get()->back(), match, patternStart);

        if (match.size() > 1) {
            jpgImages.get()->back() = jpgImages.get()->back().substr(match.position(1) + startCharacters);
        }
    }
}

std::shared_ptr<std::vector<std::string>> PdfParse::getJPGImages(std::string *pdfFile) {
    if (jpgImages.get()->size() == 0) {
        parsePDF(pdfFile);
    }
    return jpgImages;
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
