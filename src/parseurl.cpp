#include "scan2ocr.h"
#include "parseurl.h"

ParseUrl::ParseUrl (const std::string Url) {
    url = Url;
    // Url = scheme://[user]:[password]@host[:port]/directory/subdirectory/filename.extension
    int beginSearch {0};

    scheme = url.substr(0, Url.find("://"));
    beginSearch = scheme.length() + 3;

    // check if user or password is present
    if (url.find("@", beginSearch) != std::string::npos) {
        // check if user and password are present
        int searchPosition = url.find(":", beginSearch);
        if (searchPosition > 0 and searchPosition < url.find("@", beginSearch)) {
            // user and password
            username = url.substr(beginSearch, searchPosition - beginSearch);
            beginSearch = url.find(":", beginSearch) + 1;
            password = url.substr(beginSearch, url.find("@", beginSearch) - beginSearch);
            beginSearch = url.find("@", beginSearch) + 1;
        } else {
            // only user
            username = url.substr(beginSearch, url.find("@", beginSearch) - beginSearch);
            beginSearch = url.find("@", beginSearch) + 1;
        }
    }
    // Check if port is present
    if (url.find(":", beginSearch) != std::string::npos) {
        host = url.substr(beginSearch, url.find(":", beginSearch) - beginSearch);
        beginSearch = url.find(":", beginSearch) + 1;
        try
        {
            std::string ports = url.substr(beginSearch, url.find("/", beginSearch) - beginSearch);
            port = std::stoi(ports);
            beginSearch = url.find("/", beginSearch);
        }
        catch(const std::exception& e)
        {
            //Port is not numerical
        }
    }
    else if (scheme == "sftp") { // No Port and host should be present
        host = url.substr(beginSearch, url.find("/", beginSearch) - beginSearch);
        beginSearch = url.find("/", beginSearch);
    }

    // Directory is everything remaining left of the last /
    directory = url.substr(beginSearch, url.rfind("/") - beginSearch + 1);
    beginSearch = url.rfind("/") + 1;

    // Filename is everything right of the last /
    filename = url.substr(beginSearch, url.rfind("/") - beginSearch);

    // remove file extension
    rawFilename = filename.substr(0, filename.length() - 4);
}

ParseUrl::ParseUrl () {
    url = "";
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

std::string ParseUrl::getUrl () {
    url = directory + filename;
    if (scheme == "file") {
        url = "file://" + url;
    }
    else {
        // check if we have port
        if (port != 0) {
            url = ":" + std::to_string(port) + url;
        }
        url = host + url;

        // check if we have password
        if (password != "") {
            url = username + ":" + password + "@" + url;
        }
        else if (username != "") {
            url = username + "@" + url;
        }
        url = "sftp://" + url;
    }
    return url;
}
std::string ParseUrl::Url () const {
    return url;
}

QUrl ParseUrl::qUrl () const {
    return QUrl(QString::fromStdString(url));
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