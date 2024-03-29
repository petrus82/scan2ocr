#ifndef SCAN2OCR_H
#define SCAN2OCR_H

/*
Application flow:

scan2ocr::main  -> cfOpen - > OpenFile | OpenPath | Finished -> PdfFileList::addFiles ->
                                                       |            / \
                                                      \ /            |
                (ftpConnection::getConnection -> (getRemoteDir) -> getFile->disconnect)-> PdfFile::PdfFile
                -> cfMain
PdfFile::PdfFile -> removeEmptyPage -> transcodeToTiff -> ocrPdf -> getPossibleFileName 

cfMain::rename -> PdfFileList::removeFile | (ftpConnection::deleteFile)

parseUrl to encode / decode Url
global::getUniqueFileName for temp file name
*/

#define RED     "\033[31m"
#define GREEN   "\033[32m"

#include <filesystem>
#include <string>

inline namespace constants{
    constexpr const char* inputDir="/home/simon/";
    constexpr const char* sshPrivateKeyPath = "/home/simon/.ssh/";
    constexpr const char* iLanguage = "deu";
    const std::string iPathDestination = "/home/simon/Desktop/Scans";
    constexpr const float Threshold = 0.993; 
    // Get temp dir
    const std::string tmpDir = std::filesystem::temp_directory_path().string() + "/";
}

std::string getUniqueFileName();

#endif // SCAN2OCR_H


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