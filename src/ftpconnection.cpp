#include "ftpconnection.h"

FtpConnection::~FtpConnection() {
    // To be sure, disconnect everything if not already done
    disconnect();
}

FtpConnection::FtpConnection(const ParseUrl &Url) : RemoteHost(Url.Host()), Port(Url.Port()), username(Url.Username()), ftpPassword(Url.Password()) {
    
}

void FtpConnection::getConnection() {
    if (session == NULL) {
        session = ssh_new();
    }
    else {  // Reestablish connection
        if (sftp != nullptr) {
            sftp_free(sftp);
            sftp = nullptr;
        }
        ssh_disconnect(session);
        ssh_free(session);
        session = ssh_new();
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, RemoteHost.c_str());

    if (Port > 0) {
        ssh_options_set(session, SSH_OPTIONS_PORT, &Port);
    }

    Settings settings;
    ssh_options_set(session, SSH_OPTIONS_IDENTITY, settings.getSSHKeyPath().c_str());

    int rc = ssh_connect(session);

    if (rc != SSH_OK) {
        std::cerr << "Error connecting to FTP server: " << ssh_get_error(session) << std::endl;
        return;
    }

    // Verify the server's host key
    ssh_key server_pubkey;
    rc = ssh_get_server_publickey(session, &server_pubkey);
    if (rc == SSH_OK) {
        ssh_key_free(server_pubkey);
    } else {
        std::cerr << "Server's host key is not known." << std::endl;
        return;
    }

    // First try to authenticate with the server using ssh-keys
    rc = ssh_userauth_publickey_auto(session, NULL, settings.getSSHKeyPath().c_str());
    if (rc != SSH_AUTH_SUCCESS) {
        // Authentication using keys failed, try user/password
        rc = ssh_userauth_password(session, NULL, ftpPassword.c_str());
        if (rc != SSH_AUTH_SUCCESS) {
            // Also user/password authentication failed, give up
            std::cerr << "Error connecting to FTP server: " << ssh_get_error(session) << std::endl;
            return;
        } 
    }

    // Open an SFTP session
    sftp = sftp_new(session);
    rc = sftp_init(sftp);
    if (rc != SSH_OK) {
        std::cerr << "Error initializing SFTP session: " << ssh_get_error(session) << std::endl;
        return;
    }
    else {
        connected = true;
    }
}
void FtpConnection::disconnect() {
    sftp_free(sftp);
    sftp = nullptr;
    ssh_disconnect(session);
    ssh_free(session);
    session = nullptr;
}

bool FtpConnection::deleteFile(const std::string &url) {
    getConnection();

    const char *cUrl = url.c_str();

    sftp_file file = sftp_open(sftp, cUrl, O_WRONLY, 0);
    if (file == NULL) {
        std::cerr << "Error opening file: " << ssh_get_error(session) << std::endl;
        return false;
        disconnect();
    }

    int rc = sftp_unlink(sftp, cUrl);
    sftp_close(file);
    if (rc != SSH_OK) {
        std::cerr << "Error deleting file: " << ssh_get_error(session) << std::endl;
        disconnect();
        return false;
    }
    disconnect();
    return true;
}

const std::string FtpConnection::getFile(const std::string remoteFilename, const std::string remoteDirectory) {
    getConnection();

    sftp_file remoteFile = sftp_open(sftp, (remoteDirectory + "/" + remoteFilename).c_str(), O_RDONLY, 0);
    if (remoteFile == NULL) {
        std::cerr << "Error opening file: " << ssh_get_error(session) << std::endl;
        return nullptr;
    }

    std::string localFilename = getUniqueFileName();

    FILE* localFile = fopen((localFilename).c_str(), "wb");
    if (localFile == NULL) {
        std::cerr << "Error opening local file for writing" << std::endl;
        sftp_close(remoteFile);
        disconnect();
        return nullptr;
    }

    // Read the remote file and write to the local file
    char buffer[1024];
    ssize_t nbytes;
    while ((nbytes = sftp_read(remoteFile, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, nbytes, localFile);
    }

    // Check for errors
    if (nbytes < 0) {
        std::cerr << "Error reading file: " << ssh_get_error(session) << std::endl;
        disconnect();
        return nullptr;
    }

    // Close the file
    int rc = sftp_close(remoteFile);
    fclose(localFile);
    if (rc != SSH_OK) {
        std::cerr << "Error closing file: " << ssh_get_error(session) << std::endl;
        disconnect();
        return nullptr;
    }
    disconnect();
    return localFilename;
}

std::vector<sftp_attributes> FtpConnection::getRemoteDir(std::string directory) {
    sftp_dir dir;
    getConnection();

    // Open directory
    dir = sftp_opendir(sftp, directory.c_str());

    if (dir == NULL) {
        std::cerr << "Directory not found: " << directory << "\nError: " << ssh_get_error(session) << std::endl;
        disconnect();
        return std::vector<sftp_attributes>();
    }

    std::vector<sftp_attributes> filesAttrs;

    // Read directory
    sftp_attributes attributes;
    while ((attributes = sftp_readdir(sftp, dir)) != NULL) {
        filesAttrs.push_back(attributes);
    }

    // Check for errors
    if (!sftp_dir_eof(dir)) {
        std::cerr << "Can't list directory: " << ssh_get_error(session) << std::endl;
        sftp_closedir(dir);
        disconnect();
        return std::vector<sftp_attributes>();
    }

    sftp_closedir(dir);
    disconnect();
    return filesAttrs;
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