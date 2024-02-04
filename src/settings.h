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
#include <QLabel>
#include <QSettings>
#include <QDir>
#include <QMessageBox>

class Settings {

public:
    enum class ThresholdMethod {
        autoThreshold,
        adaptiveThreshold,
        threshold
    };

    enum class Language {
        deu,
        eng
    };

    struct s_networkProfile {
        std::string name;
        bool isDefault;
        ParseUrl url;
    };

    struct s_documentProfile {
        std::string name;
        bool isActive;
        Language language {Language::deu};
        int resolution {600};
        ThresholdMethod thresholdMethod {ThresholdMethod::autoThreshold};
        float thresholdValue {0.993};
    };
    Settings::s_documentProfile* activeDocumentProfile;

    struct s_ProfileElement {
        std::string Element;
        bool isNumerical;
        bool isRequired;
    };

    QSettings settings;

    std::vector<Settings::s_networkProfile> getNetworkProfiles();
    std::vector<Settings::s_documentProfile> getDocumentProfiles();

    std::string getDestinationDir();

    std::string getSSHKeyPath();

    std::string TmpDir() {
        return std::filesystem::temp_directory_path().string() + "/";
    };

    int resolution() { return activeDocumentProfile->resolution;}

    std::string language() {
        switch (activeDocumentProfile->language) {
            case Language::deu:
                return "deu";
            case Language::eng:
                return "eng";
            default:
                return "deu";
        };
    }

    ThresholdMethod thresholdMethod() { return activeDocumentProfile->thresholdMethod; }
    
    float thresholdValue() { return activeDocumentProfile->thresholdValue; }
};

class SettingsUI : public QWidget, public Settings
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
    void removeNetworkProfile();
    void setDefaultNetworkProfile();

    void changedNetworkProfile(int currentRow);

    void addDocumentProfile();
    void removeDocumentProfile();
    void setDefaultDocumentProfile();

    void changedDocumentProfile(int currentRow);

    void setStepValue(int value);
    void updateVector();

private:
    
    //UI
    void createNetworkTab();
    void createDocumentTab();
    void createPathTab();

    QDialog qdSettings {this};
    QVBoxLayout layoutDialog {&qdSettings};
    QTabWidget qtwSettings;
    QDialogButtonBox buttonBox;

    // Network Tab
    QWidget qNetworkWidget {&qtwSettings};
    QVBoxLayout layoutNetworkV {&qNetworkWidget};
    QHBoxLayout layoutNetworkH {&qNetworkWidget};
    QFormLayout layoutNetworkForm;

    QLineEdit leHost;
    QSpinBox sbPort;
    QLineEdit leDirectory;
    QLineEdit leUsername;
    QLineEdit lePassword;
    
    QListWidget lwNetworkProfiles;

    QHBoxLayout layoutNetworkButtons;
    QPushButton pbAdd;
    QPushButton pbRemove;
    QPushButton pbSetDefault;

    std::vector<s_networkProfile> networkProfiles;
    void setNetworkProfiles();

    // Document Tab
    QWidget qDocumentWidget {&qtwSettings};
    QVBoxLayout layoutDocumentV {&qDocumentWidget};
    QHBoxLayout layoutDocumentH {&qDocumentWidget};
    QFormLayout layoutDocumentForm;

    QComboBox cbLanguage;
    QSpinBox sbResolution;
    QComboBox cbThresholdMethod;
    QDoubleSpinBox sbThresholdValue;

    QListWidget lwDocumentProfiles;

    QHBoxLayout layoutDocumentButtons;
    QPushButton pbAddDocumentProfile;
    QPushButton pbRemoveDocumentProfile;
    QPushButton pbSetDefaultDocumentProfile;

    std::vector<s_documentProfile> documentProfiles;
    void setDocumentProfiles();

    // Path Tab
    QWidget qPathWidget {&qtwSettings};
    QGridLayout layoutPath {&qPathWidget};
    QLabel lblDestinationDir {tr("Destination directory:")};
    QLineEdit leDestinationDir;
    QLabel lblSSHDir {tr("SSH key path:")};
    QLineEdit leSSHDir;
    QToolButton btDestinationDir;
    QToolButton btSSHDir;

    void setPaths();

    // Validator functions
    bool validateNetworkProfile();
    void validateDocumentProfile();
    bool validatePath(const QString Path);

    void accepted();

};