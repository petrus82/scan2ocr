#include "settings.h"

/**************************************class Settings*****************************************
 * 
 * Returns the saved profile values using QSettings, if no settings are saved yet, return
 * the default values
 *
 */

Settings::Settings() {
    readValues();
}

void Settings::readValues() {
    // Clear all internal variables
    networkProfiles.clear();
    documentProfiles.clear();
    
    // Read network profiles
    settings.beginGroup("NetworkProfiles");
    QStringList profileKeys = settings.childGroups();
    for (const auto &profile : profileKeys) {
        settings.beginGroup(profile);
        Settings::networkProfile newNetworkProfile;
        newNetworkProfile.name = profile.toStdString();
        newNetworkProfile.isDefault = settings.value("isDefault").toBool();
        newNetworkProfile.documentProfileName = settings.value("documentProfileName").toString().toStdString();
        newNetworkProfile.documentProfileIndex = settings.value("documentProfileIndex").toInt();
        newNetworkProfile.url = ParseUrl(settings.value("url").toString().toStdString());
        networkProfiles.emplace_back(std::make_unique<Settings::networkProfile>(newNetworkProfile));
        settings.endGroup();
    }
    settings.endGroup();

    // Read document profiles
    settings.beginGroup("DocumentProfiles");
    profileKeys = settings.childGroups();

    std::vector<int>documentProfileOrder;
    int i = 0;
    for (i=0; i < profileKeys.size(); i++) {
        Settings::documentProfile newDocumentProfile;
        settings.beginGroup(profileKeys[i]);
        
        newDocumentProfile.name = profileKeys[i].toStdString();
        documentProfileOrder.emplace_back(settings.value("index").toInt());
        newDocumentProfile.language = static_cast<Settings::Language>(settings.value("language").toInt());
        newDocumentProfile.resolution = settings.value("resolution").toInt();
        newDocumentProfile.thresholdValue = settings.value("thresholdValue").toFloat();
        newDocumentProfile.isColored = settings.value("isColored").toBool();

        if (newDocumentProfile.name.empty()) {
            newDocumentProfile.name = "default";
            newDocumentProfile.language = Settings::Language::eng;
            newDocumentProfile.resolution = 600;
            newDocumentProfile.thresholdValue = 0.993;
            newDocumentProfile.isColored = false;    
        }
        documentProfiles.emplace_back(std::make_unique<Settings::documentProfile>(newDocumentProfile));
        settings.endGroup();
    }
    settings.endGroup();

    // Set the order of the document profiles along the index vector documentProfileOrder
    if (i>0) {
        std::sort(documentProfiles.begin(), documentProfiles.end(), [&](const std::unique_ptr<documentProfile>& a, const std::unique_ptr<documentProfile>& b) {
            return documentProfileOrder[&a - &documentProfiles[0]] < documentProfileOrder[&b - &documentProfiles[0]];
        });
    }

    // Read Paths
    settings.beginGroup("Path");
    m_destinationDir = settings.value("DestinationDir").toString();
    m_sshKeyPath = settings.value("SSHKeyPath").toString();
    settings.endGroup();

    if (m_destinationDir.isEmpty()) {
        m_destinationDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0) +"/";
    }

    if (m_sshKeyPath.isEmpty()) {
        m_sshKeyPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0) + "/.ssh/";
    }
}

void Settings::saveValues() {
    settings.clear();
    settings.sync();

    // network profiles
    settings.beginGroup("NetworkProfiles");
    for (auto &profile : networkProfiles) {
        settings.beginGroup(profile->name);
        settings.setValue("isDefault", profile->isDefault);
        settings.setValue("documentProfileName", QString::fromStdString(profile->documentProfileName));
        settings.setValue("documentProfileIndex", profile->documentProfileIndex);
        settings.setValue("url", QString::fromStdString(profile->url.Url()));
        settings.endGroup();
    }
    settings.endGroup();

    // document profiles
    settings.beginGroup("DocumentProfiles");
    for (int i=0; i<documentProfiles.size(); i++) {
        settings.beginGroup(documentProfiles[i]->name);
        settings.setValue("index", i);
        settings.setValue("language", static_cast<int>(documentProfiles[i]->language));
        settings.setValue("resolution", documentProfiles[i]->resolution);
        settings.setValue("thresholdValue", documentProfiles[i]->thresholdValue);
        settings.setValue("isColored", documentProfiles[i]->isColored);
        settings.endGroup();
        settings.sync();
    }
    settings.endGroup();
    
    // paths
    settings.beginGroup("Path");
    settings.setValue("DestinationDir", m_destinationDir);
    settings.setValue("SSHKeyPath", m_sshKeyPath);
    settings.endGroup();
}

/******************** Network Profiles *********************/
Settings::networkProfile *Settings::NetworkProfile(unsigned int index) {
    if (index < networkProfiles.size()) {
        std::cout << "NetworkProfile:Host: " << networkProfiles[index]->url.Host() << std::endl;
        return networkProfiles[index].get();
    } else {
        return &noNetworkProfile;
    }
}

void Settings::addNetworkProfile(Settings::networkProfile profile) {
    networkProfiles.emplace_back(std::make_unique<Settings::networkProfile>(profile));
}

void Settings::removeNetworkProfile(unsigned int index) {
    if (networkProfiles.size() >= index) {
        networkProfiles.erase(networkProfiles.begin() + index);
    }
}

/********************* Document Profiles ******************/
Settings::documentProfile *Settings::DocumentProfile(unsigned int index) {
    if (documentProfiles.size() > index) {
        return documentProfiles[index].get();
    } else {
        return &noDocumentProfile;
    }
}

void Settings::addDocumentProfile(Settings::documentProfile profile) {
    documentProfiles.emplace_back(std::make_unique<Settings::documentProfile>(profile));
}

void Settings::removeDocumentProfile(unsigned int index) {
    if (documentProfiles.size() >= index) {
        documentProfiles.erase(documentProfiles.begin() + index);
    }
}

void Settings::setDefaultDocumentProfile(unsigned int index) {
    if (index > 0 && index < documentProfiles.size()) {
        // Move element to front which will be the default
        std::rotate(documentProfiles.begin(), documentProfiles.begin() + index, documentProfiles.end());

    }
    // Update NetworkProfiles
    // Whatever networkProfile has a DocumentProfileIndex of index, should have DocumentProfileIndex = 0, this is indexSwitchDown
    // Whatever networkProfile has a DocumentProfileIndex of 0, needs to have a DocumentProfileIndex of index, this is indexSwitchUp
    // The indexes should not be swapped before the for loop is completed, otherwise double switching will occur
    int indexSwitchUp {0}, indexSwitchDown {0};
    for (int i=0; i<networkProfiles.size(); i++) {
        if (networkProfiles[i]->documentProfileIndex == index) {
            indexSwitchDown = i;
        }
        if (networkProfiles[i]->documentProfileIndex == 0) {
            indexSwitchUp = i;
        }
    }
    networkProfiles[indexSwitchUp]->documentProfileIndex = index;
    // Change the documentProfileNames along using a tempString
    std::string tempString = networkProfiles[indexSwitchUp]->documentProfileName;
    networkProfiles[indexSwitchUp]->documentProfileName = networkProfiles[indexSwitchDown]->documentProfileName;
    networkProfiles[indexSwitchDown]->documentProfileIndex = 0;
    networkProfiles[indexSwitchDown]->documentProfileName = tempString;
    

}

/*********************** Paths **************************/
void Settings::SSHKeyPath(const QString &dir) {
    m_sshKeyPath = dir;
}

void Settings::DestinationDir(const QString &dir) {
    m_destinationDir = dir;
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
    // Set window layout
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

    // Connect DialogButtons
    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, this, &SettingsUI::clickedOK);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &qdSettings, &QDialog::reject);

    // If dialog is closed with OK and values are checked, call SettingsUI::accepted
    QObject::connect(&qdSettings,  &QDialog::accepted, this, &SettingsUI::accepted);
}

/********************************* Network Profiles **********************************/

void SettingsUI::createNetworkTab() {
    qtwSettings.setTabText(0, tr("Network profiles"));

    qNetworkWidget.setLayout(&layoutNetworkV);
    layoutNetworkV.addLayout(&layoutNetworkH);
    
    layoutNetworkH.addWidget(&lwNetworkProfiles);
    layoutNetworkForm.addRow(tr("Host: "), &leHost);
    sbPort.setRange(1, 65535);
    sbPort.setValue(22);
    layoutNetworkForm.addRow(tr("Port: "), &sbPort);
    layoutNetworkForm.addRow(tr("Directory: "), &leDirectory);
    layoutNetworkForm.addRow(tr("Username: "), &leUsername);
    lePassword.setEchoMode(QLineEdit::PasswordEchoOnEdit);
    layoutNetworkForm.addRow(tr("Password: "), &lePassword);

    // Create DocumentProfile entries
    for (int i=0; i < settings.documentProfileCount(); i++) {
        cbDocumentProfileName.addItem(settings.DocumentProfile(i)->name.c_str());
    }
    layoutNetworkForm.addRow(tr("Document profile: "), &cbDocumentProfileName);
    layoutNetworkH.addLayout(&layoutNetworkForm);

    pbAdd.setText(tr("&Add"));
    layoutNetworkButtons.addWidget(&pbAdd);
    
    pbRemove.setText(tr("&Remove"));
    layoutNetworkButtons.addWidget(&pbRemove);

    pbSetDefault.setText(tr("Set &Default"));
    layoutNetworkButtons.addWidget(&pbSetDefault);
    
    layoutNetworkButtons.setEnabled(false);
    pbRemove.setStyleSheet("color: gray;");
    pbSetDefault.setStyleSheet("color: gray;");
    pbAdd.setEnabled(true);

    layoutNetworkV.addLayout(&layoutNetworkButtons);
    layoutNetworkH.setSpacing(20);
    qtwSettings.addTab(&qNetworkWidget, tr("Network profiles"));

    // Connect Signals of networkProfile widgets
    // The selected network profile has changed or is deselected
    QObject::connect(&lwNetworkProfiles, &QListWidget::currentRowChanged, this, &SettingsUI::changedNetworkProfile);
    // The selected network profile has been doubleclicked to edit its name
    QObject::connect(&lwNetworkProfiles, &QListWidget::itemDoubleClicked, this, &SettingsUI::editNetworkProfile);
    // The selected network profile name has been changed by the user
    QObject::connect(&lwNetworkProfiles, &QListWidget::itemChanged, this, &SettingsUI::updateNetworkProfileName);
    
    // Add Button to add new network or document profile
    QObject::connect(&pbAdd,  &QPushButton::clicked, this, &SettingsUI::addNetworkProfile);

    // Remove Button to remove selected network or document profile
    QObject::connect(&pbRemove, &QPushButton::clicked, this, &SettingsUI::removeNetworkProfile);

    // Default Button to select default network / document profile
    QObject::connect(&pbSetDefault, &QPushButton::clicked, this, &SettingsUI::setDefaultNetworkProfile);

    // qNetworkWidget has been clicked, maybe update cbDocumentProfileName needed
    QObject::connect(&qtwSettings, &QTabWidget::currentChanged, this, &SettingsUI::updateDocumentProfiles);

    // The network profile widgets have been edited, call the updateVector function
    QObject::connect(&leHost, &QLineEdit::editingFinished, this, &SettingsUI::updateVector);
    QObject::connect(&sbPort, &QSpinBox::editingFinished, this, &SettingsUI::updateVector);
    QObject::connect(&leDirectory, &QLineEdit::editingFinished, this, &SettingsUI::updateVector);
    QObject::connect(&leUsername, &QLineEdit::editingFinished, this, &SettingsUI::updateVector);
    QObject::connect(&lePassword, &QLineEdit::editingFinished, this, &SettingsUI::updateVector);    
    QObject::connect(&cbDocumentProfileName, &QComboBox::currentTextChanged, this, &SettingsUI::updateVector);

    // Load all network profiles
    loadNetworkProfiles();
}

void SettingsUI::changedNetworkProfile(int currentRow) {
    // Called if the clicks on a different network profile or the selected profile is unselected / deleted
    
    // Check if no profile is selected
    if (currentRow < 0) {
        // Disable remove and setDefault buttons and set there text color to white
        pbRemove.setEnabled(false);
        pbSetDefault.setEnabled(false);
        pbRemove.setStyleSheet("color: gray;");
        pbSetDefault.setStyleSheet("color: gray;");
        return;
    }

    // A different profile is selected, update the form
    // Enable remove and setDefault buttons
    pbRemove.setEnabled(true);
    pbSetDefault.setEnabled(true);
    pbRemove.setStyleSheet("color: black;");
    pbSetDefault.setStyleSheet("color: black;");

    // Update the other widgets to reflect the current network profile
    leHost.setText(QString::fromStdString(settings.NetworkProfile(currentRow)->url.Host()) );
    sbPort.setValue(settings.NetworkProfile(currentRow)->url.Port());
    leDirectory.setText(QString::fromStdString(settings.NetworkProfile(currentRow)->url.Directory()));
    leUsername.setText(QString::fromStdString(settings.NetworkProfile(currentRow)->url.Username()));
    lePassword.setText(QString::fromStdString(settings.NetworkProfile(currentRow)->url.Password()));
    cbDocumentProfileName.setCurrentIndex(settings.NetworkProfile(currentRow)->documentProfileIndex);
}

void SettingsUI::editNetworkProfile(QListWidgetItem *item) {
    // User has double clicked on a network profile to edit the name
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    lwNetworkProfiles.setCurrentItem(item);
    lwNetworkProfiles.editItem(item);
}

void SettingsUI::updateNetworkProfileName(QListWidgetItem* item) {
    // The user has finished changing the network profile name, update the settings vector
    const int index {lwNetworkProfiles.row(item)};
    settings.NetworkProfile(index)->name = item->text().toStdString();
}

void SettingsUI::addNetworkProfile() {
    // The user has clicked on the add button to create a new network profile

    // Create a new networkProfile QListWidgetItem
    QListWidgetItem *addedItem = new QListWidgetItem;       // Memory management by QListWidget, so new may be safe
    addedItem->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    lwNetworkProfiles.addItem(addedItem);
    // Make the new item editable to set its name
    lwNetworkProfiles.editItem(addedItem);
    // Enable the remove and set default buttons
    layoutNetworkButtons.setEnabled(true);

    // Get default Document Profile
    std::string defaultDocumentProfileName; 
    settings.networkProfileCount() > 0 ? settings.NetworkProfile(0)->documentProfileName : "default";
    
    
    // Create new networkProfile vector element with default entries
    Settings::networkProfile newProfile{
        "defaultname",
        false,
        defaultDocumentProfileName,
        0,
        ParseUrl{"sftp://Hostname:22/Directory/"}};
    settings.addNetworkProfile(newProfile);

    // If it is the first profile, make it default
    if (settings.networkProfileCount() == 1) {
        lwNetworkProfiles.item(0)->setFont(QFont("", -1, QFont::Bold));
    }
}

void SettingsUI::removeNetworkProfile() {
    // The user has clicked on the remove button
    // QListWidget throws an exception if an item is removed before another item.
    // Therefore the whole list has to be deleted and then reconstructed:
    // Remove the selected network profile, cleanup the widget and reread the content

    int index = lwNetworkProfiles.currentRow();
    settings.removeNetworkProfile(index);
    lwNetworkProfiles.clear();
    loadNetworkProfiles();
}

void SettingsUI::setDefaultNetworkProfile() {
    //User has clicked on set default button

    if (lwNetworkProfiles.currentItem() != nullptr) {
        // New default element
        int defaultNr {lwNetworkProfiles.currentRow()};

        // Remove the old default
        for (int i=0; i< lwNetworkProfiles.count(); i++) {
            if (lwNetworkProfiles.item(i) ->font().bold()) {
                lwNetworkProfiles.item(i)->setFont(QFont("", -1));
                settings.NetworkProfile(i)->isDefault = false;
            }
        }

        // Set new default
        settings.NetworkProfile(defaultNr)->isDefault = true;
        lwNetworkProfiles.item(defaultNr)->setFont(QFont("", -1, QFont::Bold));
    }
}

void SettingsUI::updateDocumentProfiles(int index) {
    enum tabIndex
    {
        netWorkTab = 0,
        documentTab = 1,
        pathTab = 2
    };

    if (index == tabIndex::netWorkTab) {
        cbDocumentProfileName.clear();
        for (int i=0; i < settings.documentProfileCount(); i++) {
            cbDocumentProfileName.addItem(QString::fromStdString(settings.DocumentProfile(i)->name));
        }
        cbDocumentProfileName.setCurrentIndex(settings.NetworkProfile(lwNetworkProfiles.currentRow())->documentProfileIndex);   
    }
}

void SettingsUI::updateVector() {
    QObject *senderObject = QObject::sender();
    int profileIndexNetwork {lwNetworkProfiles.currentRow()};
    int profileIndexDocument {lwDocumentProfiles.currentRow()};

    if(profileIndexNetwork >= 0 && profileIndexNetwork < settings.networkProfileCount()) {
        if (senderObject == &leHost) {
            settings.NetworkProfile(profileIndexNetwork)->url.Host(leHost.text().toStdString());
        }
        else if (senderObject == &sbPort) {
            settings.NetworkProfile(profileIndexNetwork)->url.Port(sbPort.value());
        }
        else if (senderObject == &leDirectory) {
            settings.NetworkProfile(profileIndexNetwork)->url.Directory(leDirectory.text().toStdString());
        }
        else if (senderObject == &leUsername) {
            settings.NetworkProfile(profileIndexNetwork)->url.Username(leUsername.text().toStdString());
        }
        else if (senderObject == &lePassword) {
            settings.NetworkProfile(profileIndexNetwork)->url.Password(lePassword.text().toStdString());
        }
        else if (senderObject == &cbDocumentProfileName) {
            settings.NetworkProfile(profileIndexNetwork)->documentProfileName = cbDocumentProfileName.currentText().toStdString();
            settings.NetworkProfile(profileIndexNetwork)->documentProfileIndex = cbDocumentProfileName.currentIndex();
        }
    }
    if (profileIndexDocument >= 0 && profileIndexDocument < settings.documentProfileCount()) {
        if (senderObject == &cbLanguage) {
            settings.DocumentProfile(profileIndexDocument)->language = static_cast<Settings::Language>(cbLanguage.currentIndex());
        }
        else if (senderObject == &sbResolution) {
            setStepValue(sbResolution.value());
            settings.DocumentProfile(profileIndexDocument)->resolution = sbResolution.value();
        }
        else if (senderObject == &sbThresholdValue) {
            settings.DocumentProfile(profileIndexDocument)->thresholdValue = sbThresholdValue.value();
        }
        else if (senderObject == &cbIsColored) {
            settings.DocumentProfile(profileIndexDocument)->isColored = cbIsColored.isChecked();
        }
    }
}

void SettingsUI::loadNetworkProfiles() {
    for (int i=0; i < settings.networkProfileCount(); i++) {
        lwNetworkProfiles.addItem(QString::fromStdString(settings.NetworkProfile(i)->name));

        if (settings.NetworkProfile(i)->isDefault) {    
            // Set the font to bold of the last added item
            lwNetworkProfiles.item(lwNetworkProfiles.count() - 1)->setFont(QFont("", -1, QFont::Bold));
            leHost.setText(QString::fromStdString(settings.NetworkProfile(i)->url.Host()));
            sbPort.setValue(settings.NetworkProfile(i)->url.Port());
            leDirectory.setText(QString::fromStdString(settings.NetworkProfile(i)->url.Directory()));
            leUsername.setText(QString::fromStdString(settings.NetworkProfile(i)->url.Username()));
            lePassword.setText(QString::fromStdString(settings.NetworkProfile(i)->url.Password()));
            loadDocumentProfile();
            cbDocumentProfileName.setCurrentIndex(settings.NetworkProfile(i)->documentProfileIndex);
        }
    }
}

bool SettingsUI::validateNetworkProfile() {
    for (int i=0; i < settings.networkProfileCount(); i++) {
        if (settings.NetworkProfile(i)->name == "default" || settings.NetworkProfile(i)->name == "") {
            int result = QMessageBox::warning(this, tr("Error"), tr("Default profile name for profile no° ") + QString::number(i) +
            ". Do you want to delete it?", QMessageBox::Yes | QMessageBox::No
            );
            if (result == QMessageBox::Yes) {
                settings.removeNetworkProfile(i);
                // Try again
                return validateNetworkProfile();
            }
            return false;
        }
        if (settings.NetworkProfile(i)->url.Host() == "Hostname") {
            QMessageBox::warning(this, tr("Error"), tr("Default hostname for profile ") + QString::fromStdString(settings.NetworkProfile(i)->name + "."));
            return false;
        }
        if (settings.NetworkProfile(i)->url.Directory() == "/Directory/") {
            QMessageBox::warning(this, tr("Error"), tr("Empty directory for profile ") + QString::fromStdString(settings.NetworkProfile(i)->name + "."));
            return false;
        }
        // Make sure the directory entry is terminated with a "/"
        std::string directory {settings.NetworkProfile(i)->url.Directory()};
        if (directory.substr(directory.length()-1, 1) != "/") {
            settings.NetworkProfile(i)->url.Directory(directory + "/");
        };
    } 
    // There is either no profile entry or it has a name and all required entries
    return true;
}

/******************************************Document Profiles*********************************/
void SettingsUI::createDocumentTab() {
    qDocumentWidget.setLayout(&layoutDocumentV);
    layoutDocumentV.addLayout(&layoutDocumentH);

    layoutDocumentH.addWidget(&lwDocumentProfiles);

    cbLanguage.addItem("Deutsch");
    cbLanguage.addItem("English");
    layoutDocumentForm.addRow(tr("Language: "), &cbLanguage);
    sbResolution.setRange(150, 1200);
    sbResolution.setKeyboardTracking(false);
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
    // The selected document profile has changed or is deselected
    QObject::connect(&lwDocumentProfiles, &QListWidget::currentRowChanged, this, &SettingsUI::changedDocumentProfile);
    // The user has double clicked on a document profile to edit its name
    QObject::connect(&lwDocumentProfiles, &QListWidget::itemDoubleClicked, this, &SettingsUI::editDocumentProfile);
    // The selected document profile name has been changed by the user
    QObject::connect(&lwDocumentProfiles, &QListWidget::itemChanged, this, &SettingsUI::updateDocumentProfileName);

    // The user has clicked on the add button
    QObject::connect(&pbAddDocumentProfile, &QPushButton::clicked, this, &SettingsUI::addDocumentProfile);
    // The user has clicked on the remove button
    QObject::connect(&pbRemoveDocumentProfile, &QPushButton::clicked, this, &SettingsUI::removeDocumentProfile);
    // The user has clicked on the set default button
    QObject::connect(&pbSetDefaultDocumentProfile, &QPushButton::clicked, this, &SettingsUI::setDefaultDocumentProfile);

    // The document profile widgets have been edited, call SettingsUI::updateVector
    //QObject::connect(&sbResolution, &QSpinBox::valueChanged, this, &SettingsUI::setStepValue);
    QObject::connect(&lwDocumentProfiles, &QListWidget::currentRowChanged, this, &SettingsUI::updateVector);
    QObject::connect(&cbLanguage, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsUI::updateVector);
    QObject::connect(&sbResolution, QOverload<int>::of(&QSpinBox::valueChanged), this, &SettingsUI::updateVector);
    QObject::connect(&sbThresholdValue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SettingsUI::updateVector);
    QObject::connect(&cbIsColored, QOverload<int>::of(&QCheckBox::stateChanged), this, &SettingsUI::updateVector);

    // Load document profiles
    loadDocumentProfile();
}

void SettingsUI::changedDocumentProfile(int index) {
    // If the user clicks on a different document profile or the selected profile is unselected / deleted

    // Check if no profile is selected
    if (index < 0) {
        // Disable remove and setDefault buttons and set there text color to white
        pbRemoveDocumentProfile.setEnabled(false);
        pbSetDefaultDocumentProfile.setEnabled(false);
        pbRemoveDocumentProfile.setStyleSheet("color: gray;");
        pbSetDefaultDocumentProfile.setStyleSheet("color: gray;");
        return;
    }

    // A different profile is selected, update the form
    // Enable remove and setDefault buttons
    pbRemoveDocumentProfile.setEnabled(true);
    pbSetDefaultDocumentProfile.setEnabled(true);
    pbRemoveDocumentProfile.setStyleSheet("color: black;");
    pbSetDefaultDocumentProfile.setStyleSheet("color: black;");

    // Update the other widgets to reflect the current document profile
    cbLanguage.setCurrentIndex(static_cast<int>(settings.DocumentProfile(index)->language));
    sbResolution.setValue(settings.DocumentProfile(index)->resolution);
    sbThresholdValue.setValue(settings.DocumentProfile(index)->thresholdValue);
    cbIsColored.setChecked(settings.DocumentProfile(index)->isColored);
}

void SettingsUI::editDocumentProfile(QListWidgetItem *item) {
    // The user has double clicked on a document profile to edit its name
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    lwDocumentProfiles.setCurrentItem(item);
    lwDocumentProfiles.editItem(item);
}

void SettingsUI::updateDocumentProfileName(QListWidgetItem* item) {
    // The user has finished changing the document profile name, update the settings vector
    const int index {lwDocumentProfiles.row(item)};
    settings.DocumentProfile(index)->name = item->text().toStdString();  
}

void SettingsUI::addDocumentProfile() {
    // The user has clicked on the add button to create a new document profile

    // Create a QListWidgetItem
    QListWidgetItem *addedItem = new QListWidgetItem;
    addedItem->setFlags(Qt::ItemIsEditable|Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    lwDocumentProfiles.addItem(addedItem);
    lwDocumentProfiles.editItem(addedItem);
    layoutDocumentButtons.setEnabled(true);

    // Add documentProfile vector element with default entries
    Settings::documentProfile newProfile{
        "defaultname",
        Settings::Language::deu,
        600,
        0.993f,
        false
    };
    settings.addDocumentProfile(newProfile);

    // If it is the first profile, make it the active one
    if (settings.documentProfileCount() == 1) {
        lwDocumentProfiles.item(0)->setFont(QFont("", -1, QFont::Bold));
    }
}

void SettingsUI::removeDocumentProfile() {
    // The user has clicked on the remove button
    int indexElement = lwDocumentProfiles.currentRow();
    settings.removeDocumentProfile(indexElement);
    lwDocumentProfiles.clear();
    loadDocumentProfile();
}

void SettingsUI::setDefaultDocumentProfile() {
    // The user has clicked on the set default button
    int indexElement = lwDocumentProfiles.currentRow();
    
    settings.setDefaultDocumentProfile(indexElement);
    lwDocumentProfiles.clear();
    loadDocumentProfile();
}

void SettingsUI::loadDocumentProfile() {
    lwDocumentProfiles.clear();
    for (int i = 0; i < settings.documentProfileCount(); i++) {
        lwDocumentProfiles.addItem(settings.DocumentProfile(i)->name.c_str());
        
        if (i == 0) {
            cbLanguage.setCurrentIndex(static_cast<int>(settings.DocumentProfile(i)->language));
            sbResolution.setValue(settings.DocumentProfile(i)->resolution);
            sbThresholdValue.setValue(settings.DocumentProfile(i)->thresholdValue);
            cbIsColored.setChecked(settings.DocumentProfile(i)->isColored);
        }
    }
}

void SettingsUI::setStepValue(int newValue) {
    // value should only take 150 - 300 - 600 - 1200
    static int previousValue = newValue;
    static int isRecursion {false};

    int diff = newValue - previousValue;
    auto checkValue = [this](int value) {
        const std::vector<int> allowedValues = {150, 300, 600, 1200};
        const std::vector<int> allowedIndices = {0, 1, 2, 3};

        // Find the index of the nearest allowed value
        int nearestIndex = 0;
        for (int i = 0; i < allowedIndices.size(); i++) {
            if (allowedValues[i] >= value) {
                nearestIndex = i;
                break;
            }
        }

        // Return the nearest allowed value
        return allowedValues[nearestIndex];    
    };

    // Store the current value as the previous value for the next change
    previousValue = newValue;
    if (diff == 0) {
        return;
    }
    else if (diff == 1 && !isRecursion) {
        isRecursion = true;
        // It has been incremented, so double the value
        const int value = (previousValue-1) * 2;
        sbResolution.setValue(checkValue(value));
        //sbResolution.setSingleStep(sbResolution.value());
    } 
    else if (diff == -1 && !isRecursion) {
        isRecursion = true;
        // It has been decremented, so halve the value
        const int value = (previousValue+1) / 2;
        sbResolution.setValue(checkValue(value));
        //sbResolution.setSingleStep(sbResolution.value()/2);
    } else if (isRecursion) {
        isRecursion = false;
    }
}
bool SettingsUI::validateDocumentProfile() {
    for (int i = 0; i < settings.documentProfileCount(); i++) {
        // Check if the profile name is empty, if so it should be deleted
        #ifdef DEBUG
            qDebug() << "Document profile name: " << settings.DocumentProfile(i)->name;
        #endif
        if (settings.DocumentProfile(i)->name == "default" || settings.DocumentProfile(i)->name == "") {
            // The user has not entered a name, maybe the profile should be deleted
            int result = QMessageBox::question(
                this, tr("Remove document profile?"), 
                tr("Document profile has number ") + QString::number(i) + tr(" has default name. Do you want to delete it?"), 
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes
            );
            if (result == QMessageBox::Yes) {
                settings.removeDocumentProfile(i);
                return true;    
            }
            else if (result == QMessageBox::Cancel) {
                return false;
            }
            return true;            
        };    
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
    QObject::connect(&btDestinationDir, &QPushButton::pressed, this, &SettingsUI::getPaths);
    QObject::connect(&btSSHDir, &QPushButton::pressed, this, &SettingsUI::getPaths);
}

void SettingsUI::getPaths() {
    QObject *senderObject = QObject::sender();
    QFileDialog fileDialog(this);

    if (senderObject == &btDestinationDir) {
        QString destinationDir = fileDialog.getExistingDirectory(this, tr("Destination directory"), leDestinationDir.text());
        if (!destinationDir.isEmpty()) {
            leDestinationDir.setText(destinationDir);
            settings.DestinationDir(destinationDir);
        }
    }
    else if (senderObject == &btSSHDir) {
        QString sshDir = fileDialog.getExistingDirectory(this, tr("SSH directory"), leSSHDir.text());
        if (!sshDir.isEmpty()) {
            leSSHDir.setText(sshDir);
            settings.SSHKeyPath(sshDir);
        }
    }
}

void SettingsUI::setPaths() {
    leDestinationDir.setText(settings.DestinationDir());
    leSSHDir.setText(settings.SSHKeyPath());
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
    settings.saveValues();
}