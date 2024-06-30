#ifndef FTPCONNECTION_H
#define FTPCONNECTION_H

#include <iostream>
#include <string>
#include <fcntl.h>
#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include "parseurl.h"
#include "settings.h"
#include "scan2ocr.h"

class FtpConnection {
    
private:
    ssh_session session {nullptr};
    sftp_session sftp {nullptr};
    std::string RemoteHost {""};
    int Port {22};
    const std::string Filename {""};
    const std::string Directory {""};
    std::string Username {""};
    std::string FtpPassword {""};
    
    void getConnection();
    void disconnect();

public:
    FtpConnection(const ParseUrl &Url);
    ~FtpConnection();

    bool deleteFile();
    bool connected = false;
    std::unique_ptr<std::string> getFilePtr();
    
    std::unique_ptr<std::vector<std::string>> getRemoteDir(bool isRecursive);
};

#endif // FTPCONNECTION_H

/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email (at) simonboettge . de

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