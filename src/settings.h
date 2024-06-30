#pragma once

#include "parseurl.h"
#include "scan2ocr.h"
#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QEvent>
#include <QDialog>
#include <QLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QListWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QSettings>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

class Settings {

public:
    enum class Language {
        deu,
        eng
    };

    Settings();
    void readValues();
    void saveValues();

    /************ Network Profiles ************/
    struct networkProfile {
        std::string name;
        bool isDefault {true};
        bool isRecursive {true};
        std::string documentProfileName;
        int documentProfileIndex;
        ParseUrl url;

        bool operator!=(const networkProfile& other) const {
            return (name != other.name);
        }
    };
    networkProfile noNetworkProfile {
        .name = "no host",
        .isDefault = false,
        .isRecursive = false,
        .documentProfileName = "",
        .documentProfileIndex = 0,
        .url = ParseUrl("file:///nohost:0/nodirectory/nofile")
    };

    Settings::networkProfile *NetworkProfile(unsigned int index);
    void addNetworkProfile(Settings::networkProfile profile);
    void removeNetworkProfile(unsigned int index);
    const int networkProfileCount() { return networkProfiles.size(); }

    /************ Document Profiles ************/
    struct documentProfile {
        std::string name {"default"};
        Language language {Language::deu};
        int resolution {600};
        float thresholdValue {0.993f};
        bool isColored {false};

        bool operator!=(const documentProfile& other) const {
            return (name != other.name);
        }
    };

    Settings::documentProfile noDocumentProfile {
        .name = "no profile",
        .language = Settings::Language::deu,
        .resolution = 600,
        .thresholdValue = 0.993f,
        .isColored = false
    };

    Settings::documentProfile *DocumentProfile(unsigned int index);
    void addDocumentProfile(Settings::documentProfile profile);
    void removeDocumentProfile(unsigned int index);
    void setDefaultDocumentProfile(unsigned int index);

    const int documentProfileCount() { return documentProfiles.size(); }

    /************ Paths ************/
    void DestinationDir(const QString &dir);
    QString DestinationDir() { return m_destinationDir; }

    void SSHKeyPath(const QString &path);
    QString SSHKeyPath() { return m_sshKeyPath; };

    std::string TmpDir() {
        return std::filesystem::temp_directory_path().string() + "/";
    };

    std::string language() {
        switch (documentProfiles[0]->language) {
            case Language::deu:
                return "deu";
            case Language::eng:
                return "eng";
            default:
                return "deu";
        };
    }

    int resolution () { return documentProfiles[0]->resolution; }
    float thresholdValue() { return documentProfiles[0]->thresholdValue; }
    bool isColored() { return documentProfiles[0]->isColored; }

private:
    QSettings settings;

    std::vector<std::unique_ptr<Settings::networkProfile>> networkProfiles;
    std::vector<std::unique_ptr<documentProfile>> documentProfiles;

    QString m_destinationDir;
    QString m_sshKeyPath;
};

class SettingsUI : public QWidget
/*
    Class to manage program settings:
    - Tab Network profiles
        - Add networkprofile
        - Remove networkprofile
        - Set default networkprofile
            - Network profile consists of:
                - Profile name
                - Host
                - Port
                - Username
                - Password
                - Directory
        The changes are stored in addedNetworkProfile, deletedNetworkProfile and defaultNetworkProfile
        which can be read from the mainWindow class to adapt the GUI. If they are empty, nothing has been done
    - Tab paths
        - default destination directory
        - ssh key path
    - Tab Document profiles
        - Add document Profile
        - Remove document profile
        - Set default document profile
            - Document profile consists of:
                - name
                - language
                - resolution
                - threshhold method
                - threshhold value
    - OK, Cancel, Apply
*/

{
    Q_OBJECT
public:

    explicit SettingsUI(QWidget *parent = nullptr);
    void showDialog();

private slots:
    void clickedOK();

    void addNetworkProfile();
    void removeNetworkProfile ();
    void setDefaultNetworkProfile();

    void editNetworkProfile(QListWidgetItem *item);
    void updateNetworkProfileName(QListWidgetItem *item);
    void changedNetworkProfile(int currentRow);

    void addDocumentProfile();
    void removeDocumentProfile();
    void setDefaultDocumentProfile();
    void editDocumentProfile(QListWidgetItem *item);
    void updateDocumentProfileName(QListWidgetItem *item);
    void updateDocumentProfiles(int index);
    void changedDocumentProfile(int currentRow);

    void setStepValue(int value);
    void updateVector();

private:
    Settings settings;
    
    //UI
    void createNetworkTab();
    void createDocumentTab();
    void createPathTab();

    QDialog qdSettings {this};
    QVBoxLayout layoutDialog {&qdSettings};
    QTabWidget qtwSettings;
    QDialogButtonBox buttonBox;

    // Network Tab
    QWidget qNetworkWidget{&qtwSettings};
    QVBoxLayout layoutNetworkV {&qNetworkWidget};
    QHBoxLayout layoutNetworkH;
    QFormLayout layoutNetworkForm;

    QLineEdit leHost;
    QSpinBox sbPort;
    QLineEdit leDirectory;
    QLineEdit leUsername;
    QLineEdit lePassword;
    QCheckBox cbRecursive;
    QComboBox cbDocumentProfileName;
    
    QListWidget lwNetworkProfiles;

    QHBoxLayout layoutNetworkButtons;
    QPushButton pbAdd;
    QPushButton pbRemove;
    QPushButton pbSetDefault;

    void loadNetworkProfiles();

    // Document Tab
    QWidget qDocumentWidget {&qtwSettings};
    QVBoxLayout layoutDocumentV {&qDocumentWidget};
    QHBoxLayout layoutDocumentH;
    QFormLayout layoutDocumentForm;

    QComboBox cbLanguage;
    QSpinBox sbResolution;
    QDoubleSpinBox sbThresholdValue;
    QCheckBox cbIsColored;

    QListWidget lwDocumentProfiles;

    QHBoxLayout layoutDocumentButtons;
    QPushButton pbAddDocumentProfile;
    QPushButton pbRemoveDocumentProfile;
    QPushButton pbSetDefaultDocumentProfile;

    void loadDocumentProfile();

    // Path Tab
    QWidget qPathWidget {&qtwSettings};
    QGridLayout layoutPath {&qPathWidget};
    QLabel lblDestinationDir {tr("Default directory where to put files:")};
    QLineEdit leDestinationDir;
    QLabel lblSSHDir {tr("Directory where to find SSH keys for passwordless login:")};
    QLineEdit leSSHDir;
    QToolButton btDestinationDir;
    QToolButton btSSHDir;

    void setPaths();
    void getPaths();
    template <typename T> void removeItem(QListWidget &listWidget, std::vector<T> &profileList);

    // Validator functions
    bool validateNetworkProfile();
    bool validateDocumentProfile();
    bool validatePath(const QString Path);

    void accepted(); // called when OK is clicked and values are checked

};

/*  scan2ocr takes a pdf file, transcodes it to TIFF G4 and assists in renaming the file.
    Copyright (C) 2024 Simon-Friedrich Böttger email (at) simonboettger . de

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