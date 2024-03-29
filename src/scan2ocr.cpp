#include "scan2ocr.h"
#include <string>
#include <chrono>
#include <random>
#include <QObject>
#include <Magick++.h>
#include "cfopen.h"
#include "cfmain.h"

std::string getUniqueFileName() {
    // Get current date and time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;

    // add random number
    std::uniform_int_distribution<int> distribution (10, 99);
    std::random_device generator;

    int randomNumber = distribution(generator);
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%H%M%S");
    std::string Filename = ss.str() + std::to_string(randomNumber) + ".pdf";
    return Filename;
}

int main (int argc,char **argv){
    Magick::InitializeMagick(*argv);

    QApplication app(argc, argv);

    cfOpen wOpen;
    wOpen.show();
    cfMain wMain;
    wMain.show();
    wOpen.setFocus();

/*  
    Testing for _parseUrl:
    {
    std::cout << "Testcase sftp://user:password@host:5003/directory1/directory2/file.pdf:" << std::endl;
    _parseUrl parseUrl("sftp://user:password@host:5003/directory1/directory2/file.pdf");
    std::string url = parseUrl.getUrl();
    std::cout << std::endl;
    }
    {
    std::cout << "Testcase sftp://host:5003/directory1/directory2/file.pdf:" << std::endl;
    _parseUrl parseUrl("sftp://host:5003/directory1/directory2/file.pdf");
    std::string url = parseUrl.getUrl();
    std::cout << std::endl;
    }
    {
    std::cout << "Testcase sftp://host/directory1/directory2/file.pdf:" << std::endl;
    _parseUrl parseUrl("sftp://host/directory1/directory2/file.pdf");
    std::string url = parseUrl.getUrl();
    std::cout << std::endl;
    }
        {
    std::cout << "Testcase sftp://user:password@host:5003/directory1/directory2/:" << std::endl;
    _parseUrl parseUrl("sftp://user:password@host:5003/directory1/directory2/");
    std::string url = parseUrl.getUrl();
    std::cout << std::endl;
    }
    {
    std::cout << "Testcase sftp://host:5003/directory1/directory2/:" << std::endl;
    _parseUrl parseUrl("sftp://host:5003/directory1/directory2/");
    std::string url = parseUrl.getUrl();
    std::cout << std::endl;
    }
    {
    std::cout << "Testcase sftp://host/directory1/directory2/file.pdf:" << std::endl;
    _parseUrl parseUrl("sftp://host/directory1/directory2/file.pdf");
    std::string url = parseUrl.getUrl();
    std::cout << std::endl;
    }
    {
    std::cout << "Testcase file:///directory1/directory2/file.pdf:" << std::endl;
    _parseUrl parseUrl("file:///directory1/directory2/file.pdf");
    std::string url = parseUrl.getUrl();
    std::cout << std::endl;
    }
    
        {
    std::cout << "Testcase file:///directory1/directory2/:" << std::endl;
    _parseUrl parseUrl("file:///directory1/directory2/");
    std::string url = parseUrl.getUrl();
    std::cout << std::endl;
    } */
    return app.exec();
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