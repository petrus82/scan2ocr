#include "parseurl.h"
#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QLayout>
#include <QPushButton>
#include <QComboBox>

class Settings : public QWidget
{
    Q_OBJECT
public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();

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
        Settings::Language language {Settings::Language::deu};
        int resolution {600};
        Settings::ThresholdMethod thresholdMethod {Settings::ThresholdMethod::autoThreshold};
        float thresholdValue {0.993};
    };

    std::vector<std::string> NetworkProfiles();
    void DeleteNetworkProfile(Settings::s_networkProfile &netProfile);
    void AddNetworkProfile(Settings::s_networkProfile &netProfile);
    void SetDefaultProfile(Settings::s_networkProfile &netProfile);

    std::string DestinationDirectory() const;
    void DestinationDirectory(const std::string &destinationDirectory);

    void AddDocumentProfile(Settings::s_documentProfile &docProfile);
    void DeleteDocumentProfile(Settings::s_documentProfile &docProfile);
    void SetDefaultDocumentProfile(Settings::s_documentProfile &docProfile);

private slots:

private:


};