#ifndef PARSEURL_H
#define PARSEURL_H

#include <string>
#include <QUrl>

class ParseUrl {
public:
    ParseUrl(const std::string Url);
    ParseUrl();
    
    std::string Scheme() const;
    void Scheme(const std::string Scheme);
    
    std::string Host() const;
    void Host(const std::string Host);

    int Port() const;
    void Port(const int Port);

    std::string Username() const;
    void Username(const std::string Username);

    std::string Password() const;
    void Password(const std::string Password);
    
    std::string Directory() const;
    void Directory(const std::string Directory);
    
    std::string FileDir () const {return directory + filename;};

    std::string Filename() const;
    void Filename(const std::string Filename);
    
    std::string RawFilename() const;
    void RawFilename(const std::string RawFilename);
    
    std::string Url();
    void Url(const std::string Url);
    
    QUrl qUrl();

private:
    std::string m_url {""};
    std::string scheme {""};
    std::string host {""};
    int port {0};
    std::string username {""};
    std::string password {""};
    std::string directory {""};
    std::string filename {""};
    std::string rawFilename {""};
};

#endif

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
