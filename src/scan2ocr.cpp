#include "scan2ocr.h"
#include <chrono>
#include <random>
#include <string>
#include <QString>
#include <QObject>
#include <QSettings>
#include <QTranslator>
#include <Magick++.h>
#include "mainwindow.h"

namespace constants {
    const std::string PathDestination = []() {
        QSettings settings;
        QString path = settings.value("PathDestination").toString();
        return path.toStdString();
    }();
}

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
    
    // Setup translations
    QTranslator translator;
    Q_UNUSED(translator.load(":/translations/german.qm"));
    QCoreApplication::installTranslator(&translator);
    QCoreApplication::setApplicationName("scan2ocr");
    #ifdef PROGRAM_VERSION
        QCoreApplication::setApplicationVersion(PROGRAM_VERSION);
    #endif
    QCoreApplication::setOrganizationName("scan2ocr");

    MainWindow mainWindow;
    mainWindow.show();
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