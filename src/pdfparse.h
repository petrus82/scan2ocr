#ifndef PDFPARSE_H
#define PDFPARSE_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>

#include "scan2ocr.h"

class PdfParse {
public:
    PdfParse();
    ~PdfParse();
    std::shared_ptr<std::vector<std::string>> getJPGImages(std::string *pdfFile);

private:
    std::shared_ptr<std::vector<std::string>> jpgImages = std::make_shared<std::vector<std::string>>();
    void parsePDF(std::string *pdfFile);

};

#endif // PDFPARSE_H

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