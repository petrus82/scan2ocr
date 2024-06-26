#ifndef SCAN2OCR_H
#define SCAN2OCR_H

#include <filesystem>
#include <string>
#include <QStandardPaths>
#include "parseurl.h"

std::string getUniqueFileName();
class MeasurePerformance {
public:
    MeasurePerformance(std::string name) : start(std::chrono::high_resolution_clock::now()), m_name(name) {
    }

    ~MeasurePerformance() {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed_time = (end - start) * 1000;
        std::cout << "Elapsed time for " << m_name << ": " << elapsed_time.count() << " ms" << std::endl;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::string m_name;
};

#endif // SCAN2OCR_H


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