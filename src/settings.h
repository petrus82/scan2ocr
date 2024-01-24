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
#include <QMessageBox>

class clickableLineEdit : public QLineEdit {
    Q_OBJECT
public:
    clickableLineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {};

    bool event (QEvent* ev) override;

signals:
    void clicked();
};

class Settings : public QWidget
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
        ParseUrl url;
    };

    struct s_documentProfile {
        Language language {Language::deu};
        int resolution {600};
        ThresholdMethod thresholdMethod {ThresholdMethod::autoThreshold};
        float thresholdValue {0.993};
    };

    struct s_ProfileElement {
        std::string Element;
        bool isNumerical;
        bool isRequired;
    };

    explicit Settings(QWidget *parent = nullptr);
    //~Settings();

    std::vector<std::string> NetworkProfiles();
    std::vector<std::string> DocumentProfiles();
    std::string DestinationDir();
    std::string SSHKeyPath();

    std::string TmpDir() {
        return std::filesystem::temp_directory_path().string() + "/";
    };

    // Construct UI
    void createNetworkTab();
    void createDocumentTab();
    void createPathTab();

    void showDialog();

private slots:

private:
    const std::string m_inputDir {QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0).toStdString().c_str()};
    const std::string m_sshPrivateKeyPath = inputDir + ".ssh/";
    
    //UI
    QDialog qdSettings {this};
    QVBoxLayout layoutDialog {&qdSettings};
    QTabWidget qtwSettings;
    QDialogButtonBox buttonBox;

    // Network Tab
    QWidget qNetworkWidget {&qtwSettings};
    QVBoxLayout layoutNetworkV {&qNetworkWidget};
    QHBoxLayout layoutNetworkH {&qNetworkWidget};
    QFormLayout layoutNetworkForm;
    clickableLineEdit leHost;
    QSpinBox sbPort;
    clickableLineEdit leDirectory;
    clickableLineEdit leUsername;
    clickableLineEdit lePassword;
    QListWidget lwNetworkProfiles;

    QHBoxLayout layoutNetworkButtons;
    QPushButton pbAdd;
    QPushButton pbRemove;
    QPushButton pbSetDefault;

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

    // Path Tab
    QWidget qPathWidget {&qtwSettings};
    QGridLayout layoutPath {&qPathWidget};
    QLabel lblDestinationDir {tr("Destination directory:")};
    QLineEdit leDestinationDir;
    QLabel lblSSHDir {tr("SSH key path:")};
    QLineEdit leSSHDir;
    QToolButton btDestinationDir;
    QToolButton btSSHDir;

};

/* class ProfileDialog : public QWidget {

    Q_OBJECT
public:
    ProfileDialog(std::vector <Scan2ocr::s_ProfileElement> profiles, QWidget *parentWidget = nullptr);
    ~ProfileDialog();

    std::vector<std::string> getInput();

private slots:
    void accept();
    void reject();

private:
    QWidget *qProfileWidget {nullptr};
    QGridLayout layout {qProfileWidget};
    
    struct sProfileElement {
        QLabel lblProfile;
        clickableLineEdit leProfile;
        bool isNumerical {false};
        bool isRequired {false};
    };

    std::vector<std::unique_ptr<sProfileElement>> profileElements;
};

class SetProfileElement : public QWidget
{
    Q_OBJECT
public:
    SetProfileElement(std::string WidgetTitle, std::vector<std::string> Elements);
    ~SetProfileElement();

    std::string getInput();

private slots:
    void accept();
    void reject();

private:
    QWidget qWidget;
    QGridLayout layout {&qWidget};
    QComboBox cbElement;
    QDialogButtonBox buttonBox {QDialogButtonBox::Ok | QDialogButtonBox::Cancel};
}; */