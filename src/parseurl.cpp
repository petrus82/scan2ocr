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
        host = m_url.substr(beginSearch, m_url.find(":", beginSearch) - beginSearch);
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
        host = m_url.substr(beginSearch, m_url.find("/", beginSearch) - beginSearch);
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
    scheme = Scheme;    
}

std::string ParseUrl::Host() const {
    return host;
}

void ParseUrl::Host (const std::string Host) {
    host = Host;
}

std::string ParseUrl::Directory () const {
    return directory;
}

void ParseUrl::Directory (const std::string Directory) {
    directory = Directory;
}

std::string ParseUrl::Filename () const {
    return filename;
}

void ParseUrl::Filename (const std::string Filename) {
    filename = Filename;
}

std::string ParseUrl::RawFilename () const {
    return rawFilename;
}

void ParseUrl::RawFilename (const std::string RawFilename) {
    rawFilename = RawFilename;
}

std::string ParseUrl::Password () const {
    return password;
}

void ParseUrl::Password (const std::string Password) {
    password = Password;
}

std::string ParseUrl::Username () const {
    return username;
}

void ParseUrl::Username (const std::string Username) {
    username = Username;
}

int ParseUrl::Port () const {
    return port;    
}

void ParseUrl::Port (const int Port) {
    port = Port;
}

std::string ParseUrl::Url () {
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
    return m_url;
}

void ParseUrl::Url (const std::string Url) {
    m_url = Url;
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