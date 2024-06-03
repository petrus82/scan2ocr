#include "scan2ocr.h"
#include "parseurl.h"

ParseUrl::ParseUrl (const std::string Url) {
    m_url = Url;
    // Url = scheme://[user]:[password]@host[:port]/directory/subdirectory/filename.extension
    int beginSearch {0};

    scheme = m_url.substr(0, Url.find("://"));
    beginSearch = scheme.length() + 3;

    // check if user or password is present
    if (m_url.find("@", beginSearch) != std::string::npos) {
        // check if user and password are present
        int searchPosition = m_url.find(":", beginSearch);
        if (searchPosition > 0 and searchPosition < m_url.find("@", beginSearch)) {
            // user and password
            username = m_url.substr(beginSearch, searchPosition - beginSearch);
            beginSearch = m_url.find(":", beginSearch) + 1;
            password = m_url.substr(beginSearch, m_url.find("@", beginSearch) - beginSearch);
            beginSearch = m_url.find("@", beginSearch) + 1;
        } else {
            // only user
            username = m_url.substr(beginSearch, m_url.find("@", beginSearch) - beginSearch);
            beginSearch = m_url.find("@", beginSearch) + 1;
        }
    }
    // Check if port is present
    if (m_url.find(":", beginSearch) != std::string::npos) {
        #ifdef DEBUG
            std::cout << "ParseUrl::host31: " << host <<", @" << &host << std::endl;
        #endif
        host = m_url.substr(beginSearch, m_url.find(":", beginSearch) - beginSearch);
        #ifdef DEBUG
            std::cout << "ParseUrl::host35: " << host <<", @" << &host << std::endl;
        #endif
        beginSearch = m_url.find(":", beginSearch) + 1;
        try
        {
            std::string ports = m_url.substr(beginSearch, m_url.find("/", beginSearch) - beginSearch);
            port = std::stoi(ports);
            beginSearch = m_url.find("/", beginSearch);
        }
        catch(const std::exception& e)
        {
            //Port is not numerical
        }
    }
    else if (scheme == "sftp") { // No Port and host should be present
        #ifdef DEBUG
            std::cout << "ParseUrl::host:51: " << host <<", @" << &host << std::endl;
        #endif
        host = m_url.substr(beginSearch, m_url.find("/", beginSearch) - beginSearch);
        #ifdef DEBUG
            std::cout << "ParseUrl::host:55: " << host <<", @" << &host << std::endl;
        #endif
        beginSearch = m_url.find("/", beginSearch);
    }

    // Directory is everything remaining left of the last /
    directory = m_url.substr(beginSearch, m_url.rfind("/") - beginSearch + 1);
    beginSearch = m_url.rfind("/") + 1;

    // Filename is everything right of the last /
    filename = m_url.substr(beginSearch, m_url.rfind("/") - beginSearch);

    // remove file extension
    rawFilename = filename.substr(0, filename.length() - 4);
}

ParseUrl::ParseUrl () {
    m_url = "";
}

std::string ParseUrl::Scheme() const {
    return scheme;
}

void ParseUrl::Scheme (const std::string Scheme) {
    if (Scheme.length() > 4){
        scheme = Scheme;    
    }
}

std::string ParseUrl::Host() const {
    return host;
}

void ParseUrl::Host (const std::string Host) {
    if (Host.length() > 0) {
        #ifdef DEBUG
            std::cout << "ParseUrl::Host:92: " << Host << std::endl;
        #endif
        host = Host;    
    }
}

std::string ParseUrl::Directory () const {
    return directory;
}

void ParseUrl::Directory (const std::string Directory) {
    if (Directory.length() > 3) {
        #ifdef DEBUG
            std::cout << "ParseUrl::Directory: Directory=" << Directory << std::endl;
        #endif
        directory = Directory;
    }
}

std::string ParseUrl::Filename () const {
    return filename;
}

void ParseUrl::Filename (const std::string Filename) {
    if (Filename.length() > 4) {
        #ifdef DEBUG
            std::cout << "ParseUrl::Filename: Filename=" << Filename << std::endl;
        #endif
        filename = Filename;
    }
}

std::string ParseUrl::RawFilename () const {
    return rawFilename;
}

void ParseUrl::RawFilename (const std::string RawFilename) {
    if (RawFilename.length() > 4) {
        #ifdef DEBUG
            std::cout << "ParseUrl::RawFilename: RawFilename=" << RawFilename << std::endl;
        #endif
        rawFilename = RawFilename;
    }
}

void ParseUrl::FileDir (const std::string FileDir) {
    int lastSlashPosition = FileDir.rfind('/');
    if (lastSlashPosition != std::string::npos) {
        directory = FileDir.substr(0, lastSlashPosition + 1);
        filename = FileDir.substr(lastSlashPosition + 1);
        rawFilename = filename.substr(0, filename.length() - 4);
        
        #ifdef DEBUG
            std::cout << "ParseUrl::FqFilename: FqFilename=" << FileDir << std::endl;
            std::cout << "ParseUrl::FqFilename: directory=" << directory << std::endl;
            std::cout << "ParseUrl::FqFilename: filename=" << filename << std::endl;
        #endif
    }
}

std::string ParseUrl::FileDir () const {
    std::string fileDir = directory + filename;
    return fileDir;
}

std::string ParseUrl::Password () const {
    return password;
}

void ParseUrl::Password (const std::string Password) {
    if (Password.length() > 0) {
        #ifdef DEBUG
            std::cout << "ParseUrl::Password: Password=" << Password << std::endl;
        #endif
        password = Password;
    }
}

std::string ParseUrl::Username () const {
    return username;
}

void ParseUrl::Username (const std::string Username) {
    if (Username.length() > 0) {
        #ifdef DEBUG
            std::cout << "ParseUrl::Username: Username=" << Username << std::endl;
        #endif
        username = Username;
    }
}

int ParseUrl::Port () const {
    return port;    
}

void ParseUrl::Port (const int Port) {
    #ifdef DEBUG
        std::cout << "ParseUrl::Port: Port=" << Port << std::endl;
    #endif
    port = Port;
}

std::string ParseUrl::Url() {
    m_url = directory + filename;
    if (scheme == "file") {
        m_url = "file://" + m_url;
    }
    else {
        // check if we have port
        if (port != 0) {
            m_url = ":" + std::to_string(port) + m_url;
        }
        m_url = host + m_url;

        // check if we have password
        if (password != "") {
            m_url = username + ":" + password + "@" + m_url;
        }
        else if (username != "") {
            m_url = username + "@" + m_url;
        }
        m_url = "sftp://" + m_url;
    }
    #ifdef DEBUG
        std::cout << "ParseUrl::Url: Url=" << m_url << std::endl;
    #endif
    return m_url;
}

void ParseUrl::Url (std::string Url) {
    if (Url.length() > 7) {
        #ifdef DEBUG
            std::cout << "ParseUrl::Url: Url=" << Url << std::endl;
        #endif
        m_url = Url;
    }
}

QUrl ParseUrl::qUrl () {
    QUrl qurl(QString::fromStdString(Url()));
    return  qurl;
}


/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email (at) simonboettger.de

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