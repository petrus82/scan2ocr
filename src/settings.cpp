#include "settings.h"

/**************************************class Settings*****************************************
 * 
 * Returns the saved profile values using QSettings, if no settings are saved yet, return
 * the default values
 *
 */

std::vector<Settings::s_networkProfile> Settings::getNetworkProfiles() {
    settings.beginGroup("NetworkProfiles");
    
    QStringList profileKeys = settings.childGroups();
    std::vector<Settings::s_networkProfile> networkProfiles;

    for (const auto &profile : profileKeys) {
        settings.beginGroup(profile);
    
        networkProfiles.emplace_back(Settings::s_networkProfile{
            profile.toStdString(),
            settings.value("isDefault").toBool(), 
            ParseUrl(settings.value("url").toString().toStdString())
            }
        );
    
        settings.endGroup();
    }
    settings.endGroup();
    
    return networkProfiles;
}

std::vector<Settings::s_documentProfile> Settings::getDocumentProfiles() {
    settings.beginGroup("DocumentProfiles");
    
    
    QStringList profileKeys = settings.childGroups();
    std::vector<s_documentProfile> documentProfiles;

    for (const auto &profile : profileKeys) {
        settings.beginGroup(profile);

        documentProfiles.emplace_back(Settings::s_documentProfile{
            profile.toStdString(),
            settings.value("isActive").toBool(),
            static_cast<Settings::Language>(settings.value("language").toInt()),
            settings.value("resolution").toInt(),
            static_cast<Settings::ThresholdMethod>(settings.value("thresholdMethod").toInt()),
            settings.value("thresholdValue").toFloat()
            }
        );
        if (documentProfiles.back().isActive) {
            activeDocumentProfile = &documentProfiles.back();
        }
        settings.endGroup();
    }
    settings.endGroup();

    if (documentProfiles.empty()) {
        documentProfiles.emplace_back(Settings::s_documentProfile{
            "default",
            false,
            Settings::Language::deu,
            600,
            Settings::ThresholdMethod::autoThreshold,
            0.993
            }
        );
        activeDocumentProfile = &documentProfiles.back();
    }
    return documentProfiles;
}

std::string Settings::getDestinationDir() {
    settings.beginGroup("Path");
    QString destinationDir {settings.value("DestinationDir").toString()};
    settings.endGroup();

    if (destinationDir.isEmpty()) {
        destinationDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0);
    }
    return destinationDir.toStdString();
}

std::string Settings::getSSHKeyPath() {
    settings.beginGroup("Path");
    QString sshKeyPath {settings.value("SSHKeyPath").toString()};
    settings.endGroup();

    if (sshKeyPath.isEmpty()) {
        sshKeyPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0) + ".ssh/";
    }
    return sshKeyPath.toStdString();
}

/********************************** class SettingsUI **********************************
* In the constructor the overall layout is created
*
* createNetworkTab, createDocumentTab and createPathTab implement the logic of the corresponding tabs
*
* If the dialog is closed with the Ok button, the user input is checked in 
* validateNetworkProfile, validateDocumentProfile and validatePath, if that succeeds,
* accepted is called which saves the settings using QSettings with the profile names as subgroups
*
* The addNetworkProfile or addDocumentProfile functions create a new entry in the ListWidget and 
* add's a new element to the private profile vector variable (networkProfiles or documentProfiles)
* with default entries. Each time a the text of an entry has been edited, the updateVector function is
* called which updates the private vector variables
*
* The remove functions clear the ListWidget items and update the private vector variable
*
* The setDefault functions set the font to normal of the previous default entry and
* set the font to bold of the new default entry
*/

SettingsUI::SettingsUI (QWidget *parent) : QWidget(parent) {
    qdSettings.setWindowTitle(tr("Settings"));
    qdSettings.setMinimumSize(600, 400);
    qtwSettings.setParent(&qdSettings);
    qtwSettings.move(20,20);
    
    createNetworkTab();
    createDocumentTab();
    createPathTab();
    
    layoutDialog.addWidget(&qtwSettings);
    buttonBox.setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layoutDialog.addWidget(&buttonBox);
    layoutDialog.setAlignment(Qt::AlignRight);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, this, &SettingsUI::clickedOK);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &qdSettings, &QDialog::reject);
    QObject::connect(&qdSettings,  &QDialog::accepted, this, &SettingsUI::accepted);
}

void SettingsUI::createNetworkTab() {
    qtwSettings.setTabText(0, tr("Network profiles"));

    qNetworkWidget.setLayout(&layoutNetworkV);
    layoutNetworkV.addLayout(&layoutNetworkH);
    
    layoutNetworkH.addWidget(&lwNetworkProfiles);
    QObject::connect(&lwNetworkProfiles, &QListWidget::currentRowChanged, this, &SettingsUI::changedNetworkProfile);

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
    QObject::connect(&pbAdd,  &QPushButton::clicked, this, &SettingsUI::addNetworkProfile);

    pbRemove.setText(tr("&Remove"));
    layoutNetworkButtons.addWidget(&pbRemove);
    QObject::connect(&pbRemove, &QPushButton::clicked, this, &SettingsUI::removeNetworkProfile);

    pbSetDefault.setText(tr("Set &Default"));
    layoutNetworkButtons.addWidget(&pbSetDefault);
    QObject::connect(&pbSetDefault, &QPushButton::clicked, this, &SettingsUI::setDefaultNetworkProfile);
    layoutNetworkButtons.setEnabled(false);
    pbAdd.setEnabled(true);

    layoutNetworkV.addLayout(&layoutNetworkButtons);
    layoutNetworkH.setSpacing(20);
    qtwSettings.addTab(&qNetworkWidget, tr("Network profiles"));

    // Connect Signals of networkProfile widgets
        QObject::connect(&leHost, &QLineEdit::textChanged, this, &SettingsUI::updateVector);
    QObject::connect(&sbPort, &QSpinBox::valueChanged, this, &SettingsUI::updateVector);
    QObject::connect(&leDirectory, &QLineEdit::textChanged, this, &SettingsUI::updateVector);
    QObject::connect(&leUsername, &QLineEdit::textChanged, this, &SettingsUI::updateVector);
    QObject::connect(&lePassword, &QLineEdit::textChanged, this, &SettingsUI::updateVector);    
}

void SettingsUI::setNetworkProfiles() {
    for (auto& profile : getNetworkProfiles()) {
        lwNetworkProfiles.addItem(QString::fromStdString(profile.name));
        networkProfiles.emplace_back(Settings::s_networkProfile{
            profile.name,
            profile.isDefault,
            profile.url}
        );

        if (profile.isDefault) {    
            // Set the font to bold of the last added item
            lwNetworkProfiles.item(lwNetworkProfiles.count() - 1)->setFont(QFont("", -1, QFont::Bold));
            leHost.setText(QString::fromStdString(profile.url.Host()));
            sbPort.setValue(profile.url.Port());
            leDirectory.setText(QString::fromStdString(profile.url.Directory()));
            leUsername.setText(QString::fromStdString(profile.url.Username()));
            lePassword.setText(QString::fromStdString(profile.url.Password()));
        }
    }
}

void SettingsUI::changedNetworkProfile(int currentRow) {
    leHost.setText(QString::fromStdString(networkProfiles[currentRow].url.Host()));
    sbPort.setValue(networkProfiles[currentRow].url.Port());
    leDirectory.setText(QString::fromStdString(networkProfiles[currentRow].url.Directory()));
    leUsername.setText(QString::fromStdString(networkProfiles[currentRow].url.Username()));
    lePassword.setText(QString::fromStdString(networkProfiles[currentRow].url.Password()));
}

void SettingsUI::addNetworkProfile() {

    // Create a new element
    lwNetworkProfiles.addItem("");
    QListWidgetItem* lastItem = lwNetworkProfiles.item(lwNetworkProfiles.count() - 1);
    lwNetworkProfiles.setCurrentItem(lastItem);
    lwNetworkProfiles.editItem(lastItem);
    layoutNetworkButtons.setEnabled(true);

    // Add networkProfile vector element with default entries
        
    networkProfiles.emplace_back(Settings::s_networkProfile{
        "defaultname",
        false,
        ParseUrl{"sftp://defaultHost/defaultDirectory"}}
    );
}

void SettingsUI::removeNetworkProfile() {
    int indexElement {lwNetworkProfiles.currentRow()};
    if (!lwNetworkProfiles.currentItem()->text().isEmpty()) {
        networkProfiles.erase(networkProfiles.begin() + indexElement);
        lwNetworkProfiles.takeItem(indexElement);
        networkProfiles.erase(networkProfiles.begin() + indexElement);
    }
    
}

void SettingsUI::setDefaultNetworkProfile() {
    if (lwNetworkProfiles.currentItem() != nullptr) {
        // New default element
        int defaultNr {lwNetworkProfiles.currentRow()};

        // Remove the old default
        for (int i=0; i< lwNetworkProfiles.count(); i++) {
            if (lwNetworkProfiles.item(i) ->font().bold()) {
                lwNetworkProfiles.item(i)->setFont(QFont("", -1));
                networkProfiles.at(i).isDefault = false;
            }
        }

        // Set new default
        networkProfiles.at(defaultNr).isDefault = true;
        lwNetworkProfiles.item(defaultNr)->setFont(QFont("", -1, QFont::Bold));
    }
}

bool SettingsUI::validateNetworkProfile() {

    // Make sure that there is no empty profile name
    QListWidgetItem *item {lwNetworkProfiles.currentItem()};
    if (item != nullptr) {
        if (!item->text().isEmpty()) {
            // Check if the user has forgotten to enter a name,
            // that would be the case if leHost, leDirectory, leUsername or lePassword contain an input
            if (leHost.text().isEmpty() || leDirectory.text().isEmpty() || leUsername.text().isEmpty() || lePassword.text().isEmpty()) {
                // There is no user input, so the current item can just be deleted
                delete item;
            } else {
                // there is some input, so the profile should have a name
                QMessageBox::warning(this, tr("Error"), tr("Please enter a profile name"));
                return false;
            }     
        } else {
            // The profile has a name, now we should check if it has all required entries
            if (leHost.text().isEmpty()) {
                QMessageBox::warning(this, tr("Error"), tr("Please enter a hostname"));
                return false;
            } else if (leDirectory.text().isEmpty()) {
                QMessageBox::warning(this, tr("Error"), tr("Please enter a directory"));
                return false;
            }
        } 
    }
    // There is either no profile entry or it has a name and all required entries
    return true;
}


void SettingsUI::createDocumentTab() {
    qDocumentWidget.setLayout(&layoutDocumentV);
    layoutDocumentV.addLayout(&layoutDocumentH);

    layoutDocumentH.addWidget(&lwDocumentProfiles);
    QObject::connect(&lwDocumentProfiles, &QListWidget::currentRowChanged, this, &SettingsUI::changedDocumentProfile);

    cbLanguage.addItem("Deutsch");
    cbLanguage.addItem("English");
    layoutDocumentForm.addRow(tr("Language: "), &cbLanguage);
    sbResolution.setRange(150, 1200);
    QObject::connect(&sbResolution, &QSpinBox::valueChanged, this, &SettingsUI::setStepValue);
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

    // Connect Signals of documentProfile widgets
    QObject::connect(&cbLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsUI::updateVector);
    QObject::connect(&sbResolution, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsUI::updateVector);
    QObject::connect(&cbThresholdMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsUI::updateVector);
    QObject::connect(&sbThresholdValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsUI::updateVector);
}

void SettingsUI::setStepValue(int newValue) {
    // value should only take 150 - 300 - 600 - 1200
    static int previousValue = newValue;
    int diff = newValue - previousValue;

    if (diff > 0) {
        // It has been incremented, so double the SingleStep value
        sbResolution.setSingleStep(sbResolution.singleStep() * 2);
    } else if (diff < 0) {
        // It has been decremented, so halve the SingleStep value
        sbResolution.setSingleStep(sbResolution.singleStep() / 2);
    }

    // Store the current value as the previous value for the next change
    previousValue = newValue;
}
void SettingsUI::setDocumentProfiles() {
    for (auto& profile : getDocumentProfiles()) {
        lwDocumentProfiles.addItem(QString::fromStdString(profile.name));
        if (profile.isActive) {    
            // Set the font to bold of the last added item
            lwDocumentProfiles.item(lwDocumentProfiles.count() - 1)->setFont(QFont("", -1, QFont::Bold));
            cbLanguage.setCurrentIndex(static_cast<int>(profile.language));
            sbResolution.setValue(profile.resolution);
            cbThresholdMethod.setCurrentIndex(static_cast<int>(profile.thresholdMethod));
            sbThresholdValue.setValue(profile.thresholdValue);
        }
    }
}

void SettingsUI::addDocumentProfile() {
    lwDocumentProfiles.addItem("");
    QListWidgetItem* lastItem = lwDocumentProfiles.item(lwDocumentProfiles.count() - 1);
    lwDocumentProfiles.setCurrentItem(lastItem);
    lwDocumentProfiles.editItem(lastItem);
    layoutDocumentButtons.setEnabled(true);

    // Add documentProfile vector element with default entries
    documentProfiles.emplace_back(Settings::s_documentProfile{
        "defaultname",
        false,
        Settings::Language::deu,
        600,
        Settings::ThresholdMethod::autoThreshold,
        0.993}
    );
}

void SettingsUI::removeDocumentProfile() {
    int indexElement {lwDocumentProfiles.currentRow()};
    if (!lwDocumentProfiles.currentItem()->text().isEmpty()) {
        documentProfiles.erase(documentProfiles.begin() + indexElement);
        lwDocumentProfiles.takeItem(indexElement);
        documentProfiles.erase(documentProfiles.begin() + indexElement);
    }
}

void SettingsUI::setDefaultDocumentProfile() {
    if (lwDocumentProfiles.currentItem() != nullptr) {
        // New default element
        int defaultNr {lwDocumentProfiles.currentRow()};

        // Remove the old default
        for (int i=0; i< lwDocumentProfiles.count(); i++) {
            if (lwDocumentProfiles.item(i) ->font().bold()) {
                lwDocumentProfiles.item(i)->setFont(QFont("", -1));
                documentProfiles.at(i).isActive = false;
            }
        }

        // Set new default
        documentProfiles.at(defaultNr).isActive = true;
        lwDocumentProfiles.item(defaultNr)->setFont(QFont("", -1, QFont::Bold));
    }
}

void SettingsUI::changedDocumentProfile(int index) {
    cbLanguage.setCurrentIndex(index);
    sbResolution.setValue(documentProfiles.at(index).resolution);
    cbThresholdMethod.setCurrentIndex(static_cast<int>(documentProfiles.at(index).thresholdMethod));
    sbThresholdValue.setValue(documentProfiles.at(index).thresholdValue);
}

void SettingsUI::validateDocumentProfile() {
    // Check if the profile name is empty, if so it should be deleted
    if (lwDocumentProfiles.currentItem()->text().isEmpty()) {
        // The user has not entered a name, we should delete the profile
        documentProfiles.erase(documentProfiles.begin() + lwDocumentProfiles.currentRow());
    };
}

bool SettingsUI::validatePath(const QString Path) {
    // Empty Path -> no modification, so return true
    if (Path.isEmpty()) {
        return true;
    }

    QDir directory(Path);
    if (!directory.exists()){
        int ret = QMessageBox::question(this, tr("Error"), tr("The ") + Path + tr(" directory does not exist. Do you want to create it?"), QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            int retVal = directory.mkpath(leDestinationDir.text());
            if (retVal == false) {
                QMessageBox::warning(this, tr("Error"), tr("The ") + Path + tr(" directory could not be created"));
                return false;
            }
            return true;
        }
        else {
            // No valid directory, the user wants to correct the path entry
            return false;
        }
    }
    return true;
}

void SettingsUI::createPathTab() {
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

void SettingsUI::setPaths() {
    leDestinationDir.setText(QString::fromStdString(getDestinationDir()));
    leSSHDir.setText(QString::fromStdString(getSSHKeyPath()));
}

void SettingsUI::updateVector() {
    QObject *senderObject = QObject::sender();
    int profileIndexNetwork {lwNetworkProfiles.currentRow()};
    int profileIndexDocument {lwDocumentProfiles.currentRow()};

    if (senderObject == &leHost) {
        networkProfiles[profileIndexNetwork].url.Host(leHost.text().toStdString());
    }
    else if (senderObject == &sbPort) {
        networkProfiles[profileIndexNetwork].url.Port(sbPort.value());
    }
    else if (senderObject == &leDirectory) {
        networkProfiles[profileIndexNetwork].url.Directory(leDirectory.text().toStdString());
    }
    else if (senderObject == &leUsername) {
        networkProfiles[profileIndexNetwork].url.Username(leUsername.text().toStdString());
    }
    else if (senderObject == &lePassword) {
        networkProfiles[profileIndexNetwork].url.Password(lePassword.text().toStdString());
    }
    else if (senderObject == &cbLanguage) {
        documentProfiles[profileIndexDocument].language = static_cast<Settings::Language>(cbLanguage.currentIndex());
    }
    else if (senderObject == &sbResolution) {
        documentProfiles[profileIndexDocument].resolution = sbResolution.value();
    }
    else if (senderObject == &cbThresholdMethod) {
        documentProfiles[profileIndexDocument].thresholdMethod = static_cast<Settings::ThresholdMethod>(cbThresholdMethod.currentIndex());
    }
    else if (senderObject == &sbThresholdValue) {
        documentProfiles[profileIndexDocument].thresholdValue = sbThresholdValue.value();
    }
}
void SettingsUI::showDialog() {
    qdSettings.exec();
}

void SettingsUI::clickedOK() {
    /* Check user inputs:
        - It could be that the user has clicked on add profile and
          has not entered a profile name, in this case the empty lwNetworkProfile item should be removed
        - The user has not entered  a hostname or directory for a profile name
        - Non existing path names of leDestinationDir and leSSHDir should be excluded
    */
    if (!validateNetworkProfile()) return;
    validateDocumentProfile();
    if (!validatePath(leDestinationDir.text())) return;
    if (!validatePath(leSSHDir.text())) return;

    qdSettings.accept();
}

void SettingsUI::accepted() {
    // Save settings
    // NetworkProfiles
    settings.beginGroup("NetworkProfiles");
    for (auto &profile : networkProfiles) {
        settings.beginGroup(profile.name);
        settings.setValue("isDefault", profile.isDefault);
        settings.setValue("url", QString::fromStdString(profile.url.Url()));
        settings.endGroup();
    }
    settings.endGroup();

    // Document Profiles
    settings.beginGroup("DocumentProfiles");
    for (auto &profile : documentProfiles) {
        settings.beginGroup(profile.name);
        settings.setValue("isDefault", profile.isActive);
        settings.setValue("language", static_cast<int>(profile.language));
        settings.setValue("resolution", profile.resolution);
        settings.setValue("thresholdMethod", static_cast<int>(profile.thresholdMethod));
        settings.setValue("thresholdValue", profile.thresholdValue);
        settings.endGroup();
    }
    settings.endGroup();

    // Path
    settings.beginGroup("Path");
    settings.setValue("DestinationDir", leDestinationDir.text());
    settings.setValue("SSHKeyPath", leSSHDir.text());
    settings.endGroup();
}