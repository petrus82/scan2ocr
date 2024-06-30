#include "ftpconnection.h"

/**
 * Destructor for the FtpConnection class.
 *
 * @param None
 *
 * @return None
 *
 * @throws None
 */
FtpConnection::~FtpConnection() {
    // To be sure, disconnect everything if not already done
    disconnect();
}

/**
 * Constructs a new FtpConnection object.
 *
 * @param Url The ParseUrl object containing the necessary information for the connection.
 *
 * @throws None
 */
FtpConnection::FtpConnection(const ParseUrl &Url) : RemoteHost(Url.Host()), 
    Port(Url.Port()), Filename(Url.Filename()), 
    Directory(Url.Directory()), 
    Username(Url.Username()), 
    FtpPassword(Url.Password()) {
        #ifdef DEBUG
            std::cout << "FtpConnection(): Connecting to " << RemoteHost << ":" << Port << Directory << Filename << std::endl;
        #endif
}

/**
 * Establishes a connection to the FTP server using SSH protocol.
 *
 * @return void
 *
 * @throws std::runtime_error if there is an error during the connection process.
 */
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
    ssh_options_set(session, SSH_OPTIONS_IDENTITY, settings.SSHKeyPath().toStdString().c_str());

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
    rc = ssh_userauth_publickey_auto(session, NULL, settings.SSHKeyPath().toStdString().c_str());
    if (rc != SSH_AUTH_SUCCESS) {
        // Authentication using keys failed, try user/password
        rc = ssh_userauth_password(session, NULL, FtpPassword.c_str());
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
/**
 * Disconnects from the FTP server and frees associated resources.
 *
 * This function releases the SFTP session and the SSH session, and sets their
 * respective pointers to null. 
 *
 * @throws None
 */
void FtpConnection::disconnect() {
    sftp_free(sftp);
    sftp = nullptr;
    ssh_disconnect(session);
    ssh_free(session);
    session = nullptr;
}

/**
 * Deletes a file on the FTP server.
 *
 * @return true if the file was successfully deleted, false otherwise
 *
 * @throws None
 */
bool FtpConnection::deleteFile() {
    getConnection();

    std::string urlString = Directory + "/" + Filename;
    const char *cUrl = urlString.c_str();

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

/**
 * Retrieves a file from the FTP server and returns its contents as a unique pointer to a string.
 *
 * @return A unique pointer to a string containing the file contents, or nullptr if an error occurs.
 *
 * @throws None
 */
std::unique_ptr<std::string> FtpConnection::getFilePtr() {
    getConnection();

    sftp_file remoteFile = sftp_open(sftp, (Directory + "/" + Filename).c_str(), O_RDONLY, 0);
    if (remoteFile == NULL) {
        std::cerr << "Error opening file: " << ssh_get_error(session) << std::endl;
        return nullptr;
    }

    // Read the remote file and write to memory object
    char buffer[1024];
    ssize_t nbytes;

    std::string ss;
    while ((nbytes = sftp_read(remoteFile, buffer, sizeof(buffer))) > 0) {
        ss.append(buffer, nbytes);
    }

    // Check for errors
    if (nbytes < 0) {
        std::cerr << "Error reading file: " << ssh_get_error(session) << std::endl;
        disconnect();
        return nullptr;
    }

    // Close the file
    int rc = sftp_close(remoteFile);
    if (rc != SSH_OK) {
        std::cerr << "Error closing file: " << ssh_get_error(session) << std::endl;
        disconnect();
        return nullptr;
    }
    disconnect();
    return std::make_unique<std::string>(std::move(ss));
}

/**
 * Retrieves the list of files in the remote directory.
 *
 * @param isRecursive Flag indicating whether to recursively search subdirectories.
 *
 * @return A unique pointer to a vector of strings containing the file paths.
 *
 * @throws None
 */
std::unique_ptr<std::vector<std::string>> FtpConnection::getRemoteDir(bool isRecursive) {
    std::unique_ptr<std::vector<std::string>> files = std::make_unique<std::vector<std::string>>();
    sftp_dir dir;
    getConnection();

    // Open base directory
    dir = sftp_opendir(sftp, Directory.c_str());

    if (dir == NULL) {
        std::cerr << "Base directory not found: " << Directory << "\nError: " << ssh_get_error(session) << std::endl;
        disconnect();
        return nullptr;
    }

    // lambda function to loop through subdirectories if recursive and add all pdf files
    std::function<void(const std::string&)> readDirectory = [&](const std::string& subDirectory) {
        sftp_dir subdir = sftp_opendir(sftp, subDirectory.c_str());
        if (subdir == NULL) {
            return;
        }

        sftp_attributes attributes;
        while ((attributes = sftp_readdir(sftp, subdir)) != NULL) {
            std::string fileName = attributes->name;

            if (fileName == "." || fileName == "..") {
                continue;
            } else if (fileName.substr(fileName.length() - 3, 3) == "pdf") {
                files->push_back(subDirectory + fileName);

                #ifdef DEBUG
                    std::cout << "FtpConnection::getRemoteDir: Found file: " << files->back() << std::endl;
                #endif
                
            } else if (attributes->permissions & S_IFDIR && isRecursive) {
                readDirectory(subDirectory + fileName);
            }
        }

        sftp_closedir(subdir);
    };
    // Start traversal from the base directory
    readDirectory(Directory);
    disconnect();
    return std::move(files);
}

/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email ( at ) simonboettger . de

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