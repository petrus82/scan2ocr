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

    for (const auto &profile : profileKeys) {
        settings.beginGroup(profile);
    
        networkProfiles.emplace_back(Settings::s_networkProfile{
            profile.toStdString(),
            settings.value("isDefault").toBool(), 
            settings.value("documentProfileName").toString().toStdString(),
            ParseUrl(settings.value("url").toString().toStdString())}
        );
    
        settings.endGroup();
    }
    settings.endGroup();
    
    return networkProfiles;
}

std::vector<Settings::s_documentProfile> Settings::getDocumentProfiles() {
    settings.beginGroup("DocumentProfiles");
    
    
    QStringList profileKeys = settings.childGroups();

    for (const auto &profile : profileKeys) {
        settings.beginGroup(profile);

        documentProfiles.emplace_back(Settings::s_documentProfile{
            profile.toStdString(),
            settings.value("isActive").toBool(),
            static_cast<Settings::Language>(settings.value("language").toInt()),
            settings.value("resolution").toInt(),
            settings.value("thresholdValue").toFloat(),
            settings.value("isColored").toBool()
            }
        );
        if (documentProfiles.back().isActive) {
            activeDocumentProfile.reset(&documentProfiles.back());
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
            0.993,
            false
            }
        );
        activeDocumentProfile.reset(&documentProfiles.back());
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

int Settings::resolution() {
    return activeDocumentProfile->resolution;
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

    // Create DocumentProfile entries
    for (auto &documentProfile : getDocumentProfiles()) {
        cbDocumentProfileName.addItem(documentProfile.name.c_str());
    }
    layoutNetworkForm.addRow(tr("Document profile: "), &cbDocumentProfileName);

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
    pbRemove.setStyleSheet("color: gray;");
    pbSetDefault.setStyleSheet("color: gray;");
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
    QObject::connect(&cbDocumentProfileName, &QComboBox::currentTextChanged, this, &SettingsUI::updateVector);

    // Load network profiles
    setNetworkProfiles();
}

void SettingsUI::setNetworkProfiles() {
    for (auto& profile : getNetworkProfiles()) {
        lwNetworkProfiles.addItem(QString::fromStdString(profile.name));
        networkProfiles.emplace_back(Settings::s_networkProfile{
            profile.name,
            profile.isDefault,
            profile.documentProfileName,
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
            cbDocumentProfileName.setCurrentText(QString::fromStdString(profile.documentProfileName));
        }
    }
}

void SettingsUI::changedNetworkProfile(int currentRow) {
    if (currentRow < 0) {
        // No profile selected
        // Disable remove and setDefault buttons and set there text color to white
        pbRemove.setEnabled(false);
        pbSetDefault.setEnabled(false);
        pbRemove.setStyleSheet("color: gray;");
        pbSetDefault.setStyleSheet("color: gray;");
        return;
    }

    // Update profile name
    networkProfiles.at(currentRow).name = lwNetworkProfiles.currentItem()->text().toStdString();

    // Enable remove and setDefault buttons
    pbRemove.setEnabled(true);
    pbSetDefault.setEnabled(true);
    pbRemove.setStyleSheet("color: black;");
    pbSetDefault.setStyleSheet("color: black;");

    // Load the corresponding profile
    leHost.setText(QString::fromStdString(networkProfiles[currentRow].url.Host()));
    sbPort.setValue(networkProfiles[currentRow].url.Port());
    leDirectory.setText(QString::fromStdString(networkProfiles[currentRow].url.Directory()));
    leUsername.setText(QString::fromStdString(networkProfiles[currentRow].url.Username()));
    lePassword.setText(QString::fromStdString(networkProfiles[currentRow].url.Password()));
}

void SettingsUI::addNetworkProfile() {

    // Create a new element
    QListWidgetItem *addedItem = new QListWidgetItem;       // Memory management by QListWidget, so new may be safe
    addedItem->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    lwNetworkProfiles.addItem(addedItem);
    lwNetworkProfiles.editItem(addedItem);
    layoutNetworkButtons.setEnabled(true);

    // Add networkProfile vector element with default entries

    // Get default Document Profile
    std::string defaultDocumentProfileName;
    for (auto &profile : getDocumentProfiles()) {
        if (profile.isActive) {
            defaultDocumentProfileName = profile.name;
            break;
        }
    }
    
    networkProfiles.emplace_back(Settings::s_networkProfile{
        "defaultname",
        false,
        defaultDocumentProfileName,
        ParseUrl{"sftp://Hostname:22/Directory/"}}
    );

    // If it is the first profile, make it default
    if (networkProfiles.size() == 1) {
        networkProfiles.at(0).isDefault = true;
        lwNetworkProfiles.item(0)->setFont(QFont("", -1, QFont::Bold));
    }
}

void SettingsUI::removeNetworkProfile() {
    int indexElement {lwNetworkProfiles.currentRow()};
    if (indexElement < 0) {
        // No profile selected
        return;
    }
    if (!lwNetworkProfiles.currentItem()->text().isEmpty()) {
        networkProfiles.erase(networkProfiles.begin() + indexElement);
        lwNetworkProfiles.takeItem(indexElement);
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
    int profileNumber {1};
    for (auto it=networkProfiles.begin(); it != networkProfiles.end(); it++) {
        if ((*it).name == "defaultname") {
            int result = QMessageBox::warning(this, tr("Error"), tr("Default profile name for profile no° ") + QString::number(profileNumber) +
            ". Do you want to delete it?", QMessageBox::Yes | QMessageBox::No
            );
            if (result == QMessageBox::Yes) {
                delete lwNetworkProfiles.item(profileNumber-1);
                networkProfiles.erase(it);
                // Try again
                return validateNetworkProfile();
            }
            return false;
        }
        if ((*it).url.Host() == "Hostname") {
            QMessageBox::warning(this, tr("Error"), tr("Default hostname for profile ") + QString::fromStdString((*it).name + "."));
            return false;
        }
        if ((*it).url.Directory() == "/Directory/") {
            QMessageBox::warning(this, tr("Error"), tr("Empty directory for profile ") + QString::fromStdString((*it).name + "."));
            return false;
        }
        // Make sure the directory entry is terminated with a "/"
        std::string directory {(*it).url.Directory()};
        if (directory.substr(directory.length()-1, 1) != "/") {
            (*it).url.Directory(directory + "/");
        };
        profileNumber++;
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

    sbThresholdValue.setDecimals(3);
    sbThresholdValue.setRange(0.001, 0.999);
    sbThresholdValue.setSingleStep(0.001);
    sbThresholdValue.setValue(0.993);
    layoutDocumentForm.addRow(tr("Threshold value: "), &sbThresholdValue);
    
    cbIsColored.setCheckState(Qt::Unchecked);
    layoutDocumentForm.addRow(tr("Preserve colors"), &cbIsColored);

    layoutDocumentH.addLayout(&layoutDocumentForm);

    pbAddDocumentProfile.setText(tr("&Add"));
    layoutDocumentButtons.addWidget(&pbAddDocumentProfile);
    pbRemoveDocumentProfile.setText(tr("&Remove"));
    layoutDocumentButtons.addWidget(&pbRemoveDocumentProfile);
    pbSetDefaultDocumentProfile.setText(tr("Set &Default"));
    layoutDocumentButtons.addWidget(&pbSetDefaultDocumentProfile);

    layoutDocumentButtons.setEnabled(false);
    pbRemoveDocumentProfile.setStyleSheet("color: gray");
    pbSetDefaultDocumentProfile.setStyleSheet("color: gray");
    pbAddDocumentProfile.setEnabled(true);

    layoutDocumentH.setSpacing(20);
    layoutDocumentV.addLayout(&layoutDocumentButtons);
    qtwSettings.addTab(&qDocumentWidget, tr("Document profiles"));

    // Connect Signals of documentProfile widgets
    QObject::connect(&lwDocumentProfiles, &QListWidget::currentRowChanged, this, &SettingsUI::updateVector);
    QObject::connect(&cbLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsUI::updateVector);
    QObject::connect(&sbResolution, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsUI::updateVector);
    QObject::connect(&sbThresholdValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsUI::updateVector);
    QObject::connect(&cbIsColored, QOverload<int>::of(&QCheckBox::stateChanged), this, &SettingsUI::updateVector);

    // Load document profiles
    setDocumentProfiles();
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
        documentProfiles.emplace_back(Settings::s_documentProfile{
            profile.name,
            profile.isActive,
            profile.language,
            profile.resolution,
            profile.thresholdValue,
            profile.isColored}
        );

        if (profile.isActive) {    
            // Set the font to bold of the last added item
            lwDocumentProfiles.item(lwDocumentProfiles.count() - 1)->setFont(QFont("", -1, QFont::Bold));
            cbLanguage.setCurrentIndex(static_cast<int>(profile.language));
            sbResolution.setValue(profile.resolution);
            sbThresholdValue.setValue(profile.thresholdValue);
            cbIsColored.setChecked(profile.isColored);
        }
    }
}

void SettingsUI::addDocumentProfile() {
    // Create a new element
    QListWidgetItem *addedItem = new QListWidgetItem;       // Memory management by QListWidget, so new may be safe
    addedItem->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    lwDocumentProfiles.addItem(addedItem);
    lwDocumentProfiles.editItem(addedItem);
    layoutDocumentButtons.setEnabled(true);

    // Add documentProfile vector element with default entries
    documentProfiles.emplace_back(Settings::s_documentProfile{
        "defaultname",
        false,
        Settings::Language::deu,
        600,
        0.993,
        false}
    );

    // If it is the first profile, make it the active one
    if (documentProfiles.size() == 1) {
        documentProfiles.at(0).isActive = true;
        lwDocumentProfiles.item(0)->setFont(QFont("", -1, QFont::Bold));
    }
}

void SettingsUI::removeDocumentProfile() {
    int indexElement {lwDocumentProfiles.currentRow()};
    if (indexElement < 0) {
        // No element selected
        return;
    }
    if (!lwDocumentProfiles.currentItem()->text().isEmpty()) {
        documentProfiles.erase(documentProfiles.begin() + indexElement);
        lwDocumentProfiles.takeItem(indexElement);
    }
}

void SettingsUI::changedDocumentProfile(int index) {
    documentProfiles.at(index).name = lwDocumentProfiles.currentItem()->text().toStdString();
    cbLanguage.setCurrentIndex(index);
    sbResolution.setValue(documentProfiles.at(index).resolution);
    sbThresholdValue.setValue(documentProfiles.at(index).thresholdValue);
    cbIsColored.setChecked(documentProfiles.at(index).isColored);
}

bool SettingsUI::validateDocumentProfile() {
    int profileNumber {0};
    for (auto it = documentProfiles.begin(); it != documentProfiles.end(); it++) {
        // Check if the profile name is empty, if so it should be deleted
        qDebug() << (*it).name;
        if ((*it).name == "defaultname") {
            // The user has not entered a name, maybe the profile should be deleted
            int result = QMessageBox::warning(
                this, tr("Error"), 
                tr("Default document profile name for profile nr° ") + 
                QString::number(profileNumber) +
                tr(". Do you want to delete it?"), 
                QMessageBox::Yes | QMessageBox::No
            );
            if (result == QMessageBox::Yes) {
                documentProfiles.erase(it);
                return true;    
            }
            return false;            
        };    
        profileNumber++;
    }
    return true;
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

    if(profileIndexNetwork>=0) {
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
        else if (senderObject == &cbDocumentProfileName) {
            networkProfiles[profileIndexNetwork].documentProfileName = cbDocumentProfileName.currentText().toStdString();
        }
    }
    if (profileIndexDocument>=0) {
        if (senderObject == &cbLanguage) {
            documentProfiles[profileIndexDocument].language = static_cast<Settings::Language>(cbLanguage.currentIndex());
        }
        else if (senderObject == &sbResolution) {
            documentProfiles[profileIndexDocument].resolution = sbResolution.value();
        }
        else if (senderObject == &sbThresholdValue) {
            documentProfiles[profileIndexDocument].thresholdValue = sbThresholdValue.value();
        }
        else if (senderObject == &cbIsColored) {
            documentProfiles[profileIndexDocument].isColored = cbIsColored.isChecked();
        }
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
    if (!validateDocumentProfile()) return;
    if (!validatePath(leDestinationDir.text())) return;
    if (!validatePath(leSSHDir.text())) return;

    qdSettings.accept();
}

void SettingsUI::accepted() {
    settings.clear();
    // Save settings
    // NetworkProfiles
    settings.beginGroup("NetworkProfiles");
    for (auto &profile : networkProfiles) {
        settings.beginGroup(profile.name);
        settings.setValue("isDefault", profile.isDefault);
        settings.setValue("url", QString::fromStdString(profile.url.Url()));
        settings.setValue("documentProfileName", QString::fromStdString(profile.documentProfileName));
        settings.endGroup();
    }
    settings.endGroup();

    // Document Profiles
    settings.beginGroup("DocumentProfiles");
    for (auto &profile : documentProfiles) {
        settings.beginGroup(profile.name);
        settings.setValue("isActive", profile.isActive);
        settings.setValue("language", static_cast<int>(profile.language));
        settings.setValue("resolution", profile.resolution);
        settings.setValue("thresholdValue", profile.thresholdValue);
        settings.setValue("isColored", profile.isColored);
        settings.endGroup();
    }
    settings.endGroup();

    // Path
    settings.beginGroup("Path");
    settings.setValue("DestinationDir", leDestinationDir.text());
    settings.setValue("SSHKeyPath", leSSHDir.text());
    settings.endGroup();
}