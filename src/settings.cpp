#include "settings.h"

bool clickableLineEdit::event (QEvent* ev) {
    if (ev->type() == QEvent::MouseButtonPress) {
        emit clicked();
        return true;
    }

    // Make sure the rest of events are handled
    return QWidget::event(ev);
}

Settings::Settings (QWidget *parent) : QWidget(parent) {
    qdSettings.setWindowTitle(tr("Settings"));
    qdSettings.setMinimumSize(600, 400);
    qtwSettings.setParent(&qdSettings);
    qtwSettings.move(20,20);
    
    createNetworkTab();
    createDocumentTab();
    createPathTab();
    
    layoutDialog.addWidget(&qtwSettings);
    buttonBox.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply);
    buttonBox.button(QDialogButtonBox::Apply)->setEnabled(false);
    layoutDialog.addWidget(&buttonBox);
    layoutDialog.setAlignment(Qt::AlignRight);
}

void Settings::createNetworkTab() {
    qtwSettings.setTabText(0, tr("Network profiles"));

    qNetworkWidget.setLayout(&layoutNetworkV);
    layoutNetworkV.addLayout(&layoutNetworkH);
    
    lwNetworkProfiles.addItem("boettger-r farbe");
    lwNetworkProfiles.addItem("boettger-r grau");
    layoutNetworkH.addWidget(&lwNetworkProfiles);

    layoutNetworkForm.addRow(tr("Host: "), &leHost);
    sbPort.setRange(1, 65535);
    sbPort.setValue(22);
    layoutNetworkForm.addRow(tr("Port: "), &sbPort);
    layoutNetworkForm.addRow(tr("Directory: "), &leDirectory);
    layoutNetworkForm.addRow(tr("Username: "), &leUsername);
    lePassword.setEchoMode(QLineEdit::PasswordEchoOnEdit);
    layoutNetworkForm.addRow(tr("Password: "), &lePassword);

    layoutNetworkH.addLayout(&layoutNetworkForm);

    pbAdd.setText(tr("&Add"));
    layoutNetworkButtons.addWidget(&pbAdd);
    pbRemove.setText(tr("&Remove"));
    layoutNetworkButtons.addWidget(&pbRemove);
    pbSetDefault.setText(tr("Set &Default"));
    layoutNetworkButtons.addWidget(&pbSetDefault);
    layoutNetworkButtons.setEnabled(false);

    layoutNetworkV.addLayout(&layoutNetworkButtons);
    layoutNetworkH.setSpacing(20);
    qtwSettings.addTab(&qNetworkWidget, tr("Network profiles"));
    
}

void Settings::createDocumentTab() {
    qDocumentWidget.setLayout(&layoutDocumentV);
    layoutDocumentV.addLayout(&layoutDocumentH);

    lwDocumentProfiles.addItem("Standard Profil");
    layoutDocumentH.addWidget(&lwDocumentProfiles);

    cbLanguage.addItem("Deutsch");
    cbLanguage.addItem("English");
    layoutDocumentForm.addRow(tr("Language: "), &cbLanguage);
    sbResolution.setRange(150, 1200);
    layoutDocumentForm.addRow(tr("Resolution: "), &sbResolution);

    cbThresholdMethod.addItem("autoThreshold");
    cbThresholdMethod.addItem("adaptiveThreshold");
    cbThresholdMethod.addItem("Threshold");
    layoutDocumentForm.addRow(tr("Threshold method: "), &cbThresholdMethod);
    sbThresholdValue.setDecimals(3);
    sbThresholdValue.setRange(0.001, 0.999);
    sbThresholdValue.setSingleStep(0.001);
    sbThresholdValue.setValue(0.993);
    layoutDocumentForm.addRow(tr("Threshold value: "), &sbThresholdValue);
    layoutDocumentH.addLayout(&layoutDocumentForm);

    pbAddDocumentProfile.setText(tr("&Add"));
    layoutDocumentButtons.addWidget(&pbAddDocumentProfile);
    pbRemoveDocumentProfile.setText(tr("&Remove"));
    layoutDocumentButtons.addWidget(&pbRemoveDocumentProfile);
    pbSetDefaultDocumentProfile.setText(tr("Set &Default"));
    layoutDocumentButtons.addWidget(&pbSetDefaultDocumentProfile);
    layoutDocumentButtons.setEnabled(false);
    layoutDocumentH.setSpacing(20);
    layoutDocumentV.addLayout(&layoutDocumentButtons);
    qtwSettings.addTab(&qDocumentWidget, tr("Document profiles"));
}

void Settings::createPathTab() {
    qPathWidget.setLayout(&layoutPath);
    layoutPath.addWidget(&lblDestinationDir, 0, 0);
    layoutPath.addWidget(&leDestinationDir, 0, 1);
    btDestinationDir.setText("...");
    layoutPath.addWidget(&btDestinationDir, 0, 2);

    layoutPath.addWidget(&lblSSHDir, 1, 0);
    layoutPath.addWidget(&leSSHDir, 1, 1);
    btSSHDir.setText("...");
    layoutPath.addWidget(&btSSHDir, 1, 2);

    qtwSettings.addTab(&qPathWidget, tr("Path settings"));
}

void Settings::showDialog() {
    qdSettings.exec();
}

/* Settings::Settings(QWidget *parent) : QWidget(parent) {
    tabWidget.addTab(&netWorkTab, tr("Network profiles"));
    tabWidget.addTab(&documentTab, tr("Document profiles"));
    tabWidget.addTab(&pathTab, tr("Path settings"));
}
std::vector<std::string> Settings::NetworkProfiles() {
    QSettings settings;
    settings.beginGroup("NetworkProfiles");
    QStringList profileKeys = settings.allKeys();
    settings.endGroup();
    std::vector<std::string> profiles;
    for (auto &it : profileKeys) {
        profiles.emplace_back(it.toStdString());
    }
    return profiles;
}

void Settings::AddNetworkProfile() {

    std::vector <Scan2ocr::s_ProfileElement> profiles;  
    
    // Elementname, isNumerical, isRequired
    profiles.emplace_back (tr("Profilename: "), true, true);
    profiles.emplace_back (tr("Host: "), false, true);
    profiles.emplace_back (tr("Port: "), true, false);
    profiles.emplace_back (tr("Username: "), false, false);
    profiles.emplace_back (tr("Directory: "), false, true);
    profiles.emplace_back (tr("Password: "), false, false);
    ProfileDialog addDialog(profiles, &netWorkTab);

    std::vector<std::string> results = addDialog.getInput();
    ParseUrl url;
    QSettings settings;
    url.Scheme("sftp");
    url.Host(results[1]);
    url.Port(std::stoi(results[2]));
    url.Directory(results[3]);
    url.Username(results[4]);
    url.Password(results[5]);

    settings.beginGroup("NetworkProfiles");
    QString keyName {QString::fromStdString(results[0])};
    settings.setValue(keyName, url.qUrl());
    settings.endGroup();

    m_addedNetworkProfile.name = results[0];
    m_addedNetworkProfile.url = url;
}

void Settings::DeleteNetworkProfile() {

     m_deletedNetworkProfile.name = results[0];
    m_deletedNetworkProfile.url = url;
}

void Settings::SetDefaultNetworkProfile() {
    
}
void Settings::AddDocumentProfile() {
    std::vector <Scan2ocr::s_ProfileElement> profiles;

    // Elementname, isNumerical, isRequired
    profiles.emplace_back (tr("Profile name: "), true, true);
    profiles.emplace_back (tr("Language: "), false, true);
    profiles.emplace_back (tr("Resolution: "), true, true);
    profiles.emplace_back (tr("Threshold method: "), true, true);
    profiles.emplace_back (tr("Threshold value: "), true, true);
    ProfileDialog addDialog(profiles, &documentTab);

    std::vector<std::string> results = addDialog.getInput();
    QSettings settings;
    settings.beginGroup("DocumentProfiles");
    QString keyName {QString::fromStdString(results[0])};

    for (int i=1; i<results.size(); i++) {
        settings.setValue(keyName, QString::fromStdString(results[i]));
    }
    settings.endGroup();
}

void Settings::DeleteDocumentProfile() {
    
}

void Settings::SetDefaultDocumentProfile() {
    
} 

ProfileDialog::ProfileDialog (std::vector<Scan2ocr::s_ProfileElement> profiles, QWidget *parentWidget) : QWidget() {
    qProfileWidget = parentWidget;
    layout.setContentsMargins(20, 20, 20, 20);

    for (const auto& profile : profiles) {
        // Create a new sProfileElement instance for each profile using std::make_unique
        auto newProfileElement = std::make_unique<sProfileElement>();
        
        // Initialize the QLabel and clickableLineEdit members of the newProfileElement
        newProfileElement->lblProfile.setText(QString::fromStdString(profile.Element));
        newProfileElement->isNumerical = profile.isNumerical;
        newProfileElement->isRequired = profile.isRequired;

        // Add the newProfileElement to the profileElements vector and the layout
        profileElements.push_back(std::move(newProfileElement));
        layout.addWidget(&profileElements.back()->lblProfile, profileElements.size(), 1);
        layout.addWidget(&profileElements.back()->leProfile, profileElements.size(), 2);
    }

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&buttonBox, profileElements.size() + 1, 1);

    connect(&buttonBox, SIGNAL(accepted()), qProfileWidget, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), qProfileWidget, SLOT(reject()));
}

ProfileDialog::~ProfileDialog() {
    disconnect(qProfileWidget, SIGNAL(accepted()), qProfileWidget, SLOT(accept()));
    disconnect(qProfileWidget, SIGNAL(rejected()), qProfileWidget, SLOT(reject()));
}

std::vector<std::string> ProfileDialog::getInput() {
    std::vector<std::string> input;

    for (auto &it : profileElements) {
        input.emplace_back(it->leProfile.text().toStdString());
    }
    return input;
}

void ProfileDialog::accept() {
    // Check if numbers are valid numbers and all required fields are filled
        for (auto &it : profileElements) {
            if (it->isNumerical) {
                try {
                    std::stoi(it->leProfile.text().toStdString());
                }   
                catch (const std::invalid_argument& e) {
                    QMessageBox::warning(this, tr("Error"), tr("Invalid ") + it->lblProfile.text() + tr(" number"));
                    QPalette Color {Qt::red};
                    it->lblProfile.setPalette(Color);
                }
            }
            if (it->isRequired && it->leProfile.text().isEmpty()) {
                QMessageBox::warning(this, tr("Error"), tr("Field ") + it->lblProfile.text() + tr(" is required"));
                QPalette Color {Qt::red};
                it->lblProfile.setPalette(Color);
            }
        }
}

void ProfileDialog::reject() {
    
}

SetProfileElement::SetProfileElement(std::string DialogTitle, std::vector<std::string> Elements) : QWidget() {
    QTabWidget *tabWidget = qobject_cast<QTabWidget*>(this->parentWidget());
    if (tabWidget) {
        int index = tabWidget->indexOf(this);
        if (index != -1) {
            tabWidget->setTabText(index, QString::fromStdString(DialogTitle));
        }
    }

    for (auto &it : Elements) {
        cbElement.addItem(QString::fromStdString(it));
    }
    layout.addWidget(&cbElement, 0, 0);
    layout.addWidget(&buttonBox, 1, 0);

    connect(&buttonBox, SIGNAL(accepted()), &qWidget, SLOT(accept()));
    connect(&buttonBox, SIGNAL(rejected()), &qWidget, SLOT(reject()));
}

SetProfileElement::~SetProfileElement() {
    disconnect(&qWidget, SIGNAL(accepted()), &qWidget, SLOT(accept()));
    disconnect(&qWidget, SIGNAL(rejected()), &qWidget, SLOT(reject()));
}

void SetProfileElement::accept() {
    
}

std::string SetProfileElement::getInput() {
    return cbElement.currentText().toStdString();
}

void SetProfileElement::reject() {
    
}*/