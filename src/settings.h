#include "parseurl.h"
#include "scan2ocr.h"
#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QEvent>
#include <QDialog>
#include <QLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
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
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

    Scan2ocr::s_networkProfile addedNetworkProfile() {
        return m_addedNetworkProfile;
    }

    Scan2ocr::s_networkProfile deletedNetworkProfile() {
        return m_deletedNetworkProfile;
    }

    Scan2ocr::s_networkProfile defaultNetworkProfile() {
        return m_defaultNetworkProfile;
    }
    //std::vector<std::string> NetworkProfiles();

    //std::string DestinationDirectory() const;
    //void DestinationDirectory(const std::string &destinationDirectory);

    //std::vector<std::string> DocumentProfiles();

private slots:

private:

    QTabWidget tabWidget;
    QWidget netWorkTab;
    QWidget pathTab;
    QWidget documentTab;

    void AddNetworkProfile();
    Scan2ocr::s_networkProfile m_addedNetworkProfile;
    void DeleteNetworkProfile();
    Scan2ocr::s_networkProfile m_deletedNetworkProfile;
    void SetDefaultNetworkProfile();
    Scan2ocr::s_networkProfile m_defaultNetworkProfile;

    void AddDocumentProfile();
    void DeleteDocumentProfile();
    void SetDefaultDocumentProfile();
};

class ProfileDialog : public QWidget {

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
};