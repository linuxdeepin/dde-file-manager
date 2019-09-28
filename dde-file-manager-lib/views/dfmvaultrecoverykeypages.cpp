/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dfmvaultrecoverykeypages.h"

#include "singleton.h"
#include "gvfs/secretmanager.h"
#include "cryptoutils.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QFileDialog>

#include <DFloatingButton>
#include <DPasswordEdit>
#include <QSaveFile>

DFM_BEGIN_NAMESPACE

VaultAskCreateKeyPage::VaultAskCreateKeyPage(QWidget *parent)
    : QWidget (parent)
{
    QLabel * description = new QLabel(tr("Do you want to export a key file in case that you forgot the password?"), this);
    description->setAlignment(Qt::AlignHCenter);

    m_nextButton = new QPushButton(tr("Export key"), this);
    m_skipButton = new QPushButton(tr("Skip"), this);

    DIconButton * icon = new DIconButton(this);
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_lock"));
    icon->setIconSize(QSize(64, 64));
    icon->setWindowFlags(Qt::WindowTransparentForInput);
    icon->setFocusPolicy(Qt::NoFocus);
    icon->setMinimumHeight(64);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(icon);
    layout->addWidget(description);
    layout->addStretch();
    layout->addWidget(m_nextButton);
    layout->addWidget(m_skipButton);

    connect(m_nextButton, &QAbstractButton::clicked, this, &VaultAskCreateKeyPage::next);
    connect(m_skipButton, &QAbstractButton::clicked, this, &VaultAskCreateKeyPage::skip);
}

VaultAskCreateKeyPage::~VaultAskCreateKeyPage()
{

}

void VaultAskCreateKeyPage::next()
{
    if (!Singleton<SecretManager>::instance()->lookupVaultPassword().isEmpty()) {
        emit requestRedirect(VaultController::makeVaultUrl("/generated_key", "recovery_key"));
    } else {
        emit requestRedirect(VaultController::makeVaultUrl("/verify", "recovery_key"));
    }
}

void VaultAskCreateKeyPage::skip()
{
    if (!Singleton<SecretManager>::instance()->lookupVaultPassword().isEmpty()) {
        Singleton<SecretManager>::instance()->clearVaultPassword();
    }
    emit requestRedirect(VaultController::makeVaultUrl());
}

// ----------------------------------------------

VaultVerifyUserPage::VaultVerifyUserPage(QWidget *parent)
    : QWidget (parent)
{
    QLabel * description = new QLabel(tr("Enter the vault password"), this);
    description->setAlignment(Qt::AlignHCenter);

    m_unlockButton = new DFloatingButton(DStyle::SP_UnlockElement, this);
    m_passwordEdit = new DPasswordEdit(this);

    DIconButton * icon = new DIconButton(this);;
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_lock"));
    icon->setIconSize(QSize(64, 64));
    icon->setWindowFlags(Qt::WindowTransparentForInput);
    icon->setFocusPolicy(Qt::NoFocus);
    icon->setMinimumHeight(64);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(icon);
    layout->addWidget(description);
    layout->addWidget(m_passwordEdit);
    layout->addWidget(m_unlockButton);
    layout->addStretch();

    layout->setAlignment(m_unlockButton, Qt::AlignHCenter);

    connect(m_unlockButton, &QAbstractButton::clicked, this, &VaultVerifyUserPage::unlock);
}

VaultVerifyUserPage::~VaultVerifyUserPage()
{

}

void VaultVerifyUserPage::unlock()
{
    m_unlockButton->setDisabled(true);
    VaultController::lockVault();
    DSecureString passwordString(m_passwordEdit->text());
    bool succ = VaultController::unlockVault(passwordString);
    if (succ) {
        // save password to GNOME keyring so we can use it later.
        Singleton<SecretManager>::instance()->storeVaultPassword(passwordString);
        m_passwordEdit->clear();
        emit requestRedirect(VaultController::makeVaultUrl("/generated_key", "recovery_key"));
    }
    m_unlockButton->setDisabled(false);
}

// ----------------------------------------------

VaultGeneratedKeyPage::VaultGeneratedKeyPage(QWidget *parent)
    : QWidget (parent)
{
    QVBoxLayout * layout = new QVBoxLayout(this);

    DIconButton * icon = new DIconButton(this);
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_lock"));
    icon->setIconSize(QSize(64, 64));
    icon->setWindowFlags(Qt::WindowTransparentForInput);
    icon->setFocusPolicy(Qt::NoFocus);
    icon->setMinimumHeight(64);

    QLabel * title = new QLabel(tr("Find your recovery key below") + '\n' +
                                tr("Take good care of the recovery key by printing, writing down or saving it to a USB flash drive"), this);
    QLabel * description = new QLabel(tr("Your recovery key is as important as your password. Do not save the key file on this computer"), this);
    title->setAlignment(Qt::AlignHCenter);
    title->setWordWrap(true);
    description->setAlignment(Qt::AlignHCenter);
    description->setWordWrap(true);
    QFont font = title->font();
    font.setBold(true);
    title->setFont(font);

    m_generatedKeyEdit = new QPlainTextEdit(this);
    m_generatedKeyEdit->setReadOnly(true);

    m_saveFileButton = new QPushButton(tr("Save"), this);
    m_finishButton = new QPushButton(tr("Done"), this);

    layout->addStretch();
    layout->addWidget(icon);
    layout->addWidget(title);
    layout->addWidget(description);
    layout->addWidget(m_generatedKeyEdit);
    layout->addStretch();
    layout->addWidget(m_saveFileButton);
    layout->addWidget(m_finishButton);

    connect(m_saveFileButton, &QAbstractButton::clicked, this, &VaultGeneratedKeyPage::saveKeyFile);
    connect(m_finishButton, &QAbstractButton::clicked, this, &VaultGeneratedKeyPage::finishButtonPressed);
}

VaultGeneratedKeyPage::~VaultGeneratedKeyPage()
{
    clearData();
}

void VaultGeneratedKeyPage::startKeyGeneration()
{
    DSecureString password = Singleton<SecretManager>::instance()->lookupVaultPassword();

    QFile encryptedFile(VaultController::makeVaultLocalPath("dde-vault.config", "vault_encrypted"));
    encryptedFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QDataStream fileStream(&encryptedFile);

    CryptoUtils::byte key[CryptoUtils::AES_128_KEY_SIZE], iv[CryptoUtils::AES_128_BLOCK_SIZE];
    CryptoUtils::secure_string origStr(password.toLatin1()), encryptedStr;
    CryptoUtils::init_aes_128_cipher();
    CryptoUtils::gen_aes_128_params(key, iv);
    CryptoUtils::aes_128_encrypt(key, iv, origStr, encryptedStr);

    int vaultConfigVersion = 1;
    QByteArray encryptedData(encryptedStr.data(), encryptedStr.length());
    QByteArray ivData((char *)iv, CryptoUtils::AES_128_BLOCK_SIZE);

    fileStream << vaultConfigVersion << ivData << encryptedData;

    encryptedFile.flush();
    encryptedFile.close();

    DSecureString ivHexStr(CryptoUtils::bin_to_hex(iv, CryptoUtils::AES_128_BLOCK_SIZE).data());
    DSecureString keyHexStr(CryptoUtils::bin_to_hex(key, CryptoUtils::AES_128_KEY_SIZE).data());

    m_generatedKeyEdit->setPlainText(createRecoveryKeyString(ivHexStr, keyHexStr));
}

void VaultGeneratedKeyPage::saveKeyFile()
{
    QString savePath = QFileDialog::getSaveFileName(this, tr("Save your vault recovery key"),
                                                    QDir::home().absoluteFilePath(tr("File Vault Recovery Key")),
                                                    tr("File Vault Recovery Key (*.txt)"));
    if (savePath.isEmpty()) return;

    QSaveFile file(savePath);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream fileStream(&file);
    fileStream << m_generatedKeyEdit->toPlainText();
    file.commit();
}

void VaultGeneratedKeyPage::finishButtonPressed()
{
    clearData();
    emit requestRedirect(VaultController::makeVaultUrl());
}

void VaultGeneratedKeyPage::clearData()
{
    Singleton<SecretManager>::instance()->clearVaultPassword();
    m_generatedKeyEdit->clear();
}

DSecureString VaultGeneratedKeyPage::createRecoveryKeyString(const DSecureString &ivHexString, const DSecureString &keyHexString)
{
    return tr("File Vault Recovery Key") + '\n' + '\n' +
           tr("To verify that this is the correct recovery key, compare the following key ID with the key ID displayed on your PC.") + '\n' +
           tr("Key ID: %1").arg(ivHexString) + '\n' + '\n' +
           tr("If they are identical, then use the following key to retrieve your vault password.") + '\n' +
           tr("Recovery Key: %1").arg(keyHexString) + '\n' + '\n' +
           tr("If they do not match, then this is not the right key, please try another recovery key.") + '\n';
}

// ----------------------------------------------

VaultVerifyRecoveryKeyPage::VaultVerifyRecoveryKeyPage(QWidget *parent)
    : QWidget (parent)
{
    QVBoxLayout * layout = new QVBoxLayout(this);

    DIconButton * icon = new DIconButton(this);
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_lock"));
    icon->setIconSize(QSize(64, 64));
    icon->setWindowFlags(Qt::WindowTransparentForInput);
    icon->setFocusPolicy(Qt::NoFocus);
    icon->setMinimumHeight(64);

    QLabel * title = new QLabel(tr("Compare the following text with the key ID in your recovery key file") + '\n' +
                                tr("If they are identical, input the recovery key below to retrieve your vault password"), this);
    title->setAlignment(Qt::AlignHCenter);
    title->setWordWrap(true);
    QFont font = title->font();
    font.setBold(true);
    title->setFont(font);

    m_verifyKeyEdit = new QLineEdit(this);
    m_verifyKeyEdit->setReadOnly(true);
    m_verifyKeyEdit->setFocusPolicy(Qt::NoFocus);
    m_recoveryKeyEdit = new QLineEdit(this);

    m_retrievePasswordButton = new QPushButton(tr("Retrieve password"), this);

    layout->addStretch();
    layout->addWidget(icon);
    layout->addWidget(title);
    layout->addWidget(m_verifyKeyEdit);
    layout->addWidget(m_recoveryKeyEdit);
    layout->addStretch();
    layout->addWidget(m_retrievePasswordButton);

    connect(m_retrievePasswordButton, &QAbstractButton::clicked, this, &VaultVerifyRecoveryKeyPage::startVerifyKey);
}

VaultVerifyRecoveryKeyPage::~VaultVerifyRecoveryKeyPage()
{

}

void VaultVerifyRecoveryKeyPage::preparePage()
{
    QFile encryptedFile(VaultController::makeVaultLocalPath("dde-vault.config", "vault_encrypted"));
    if (!encryptedFile.exists()) {
        emit requestRedirect(VaultController::makeVaultUrl());
    }

    encryptedFile.open(QIODevice::ReadOnly);
    QDataStream stream(&encryptedFile);

    int version;
    stream >> version;
    stream >> m_ivData;
    stream >> m_encryptedPasswordData;

    encryptedFile.close();

    CryptoUtils::secure_string ivHexStr = CryptoUtils::bin_to_hex((CryptoUtils::byte *)m_ivData.data(), m_ivData.length());
    m_verifyKeyEdit->setText(ivHexStr.data());
}

void VaultVerifyRecoveryKeyPage::startVerifyKey()
{
    m_retrievePasswordButton->setDisabled(true);
    bool succ = verifyKey();
    if (succ) {
        emit requestRedirect(VaultController::makeVaultUrl("/password", "recovery_key"));
    }
    m_retrievePasswordButton->setDisabled(false);
}

bool VaultVerifyRecoveryKeyPage::verifyKey()
{
    DSecureString encryptedKeyHexString(m_recoveryKeyEdit->text().trimmed());
    if (!encryptedKeyHexString.isEmpty() && !m_verifyKeyEdit->text().isEmpty()) {
        if (encryptedKeyHexString.length() != CryptoUtils::AES_128_KEY_SIZE * 2) {
            return false;
        }
        CryptoUtils::byte encryptedKeyData[CryptoUtils::AES_128_KEY_SIZE];
        CryptoUtils::secure_string keyHexStr(encryptedKeyHexString.toLatin1()),
                                   encryptedPasswordStr(m_encryptedPasswordData.data()),
                                   retrievedPasswordStr;
        CryptoUtils::hex_to_bin(keyHexStr, encryptedKeyData, CryptoUtils::AES_128_KEY_SIZE);
        CryptoUtils::aes_128_decrypt(encryptedKeyData, (CryptoUtils::byte *)m_ivData.data(),
                                     encryptedPasswordStr, retrievedPasswordStr);

        DSecureString password(retrievedPasswordStr.data());
        Singleton<SecretManager>::instance()->storeVaultPassword(password);

        return true;
    }

    return false;
}

// ----------------------------------------------

VaultPasswordPage::VaultPasswordPage(QWidget *parent)
    : QWidget (parent)
{
    QLabel * description = new QLabel(tr("Here is your vault password"), this);
    description->setAlignment(Qt::AlignHCenter);

    m_finishButton = new QPushButton(tr("OK"), this);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setReadOnly(true);

    DIconButton * icon = new DIconButton(this);
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_lock"));
    icon->setIconSize(QSize(64, 64));
    icon->setWindowFlags(Qt::WindowTransparentForInput);
    icon->setFocusPolicy(Qt::NoFocus);
    icon->setMinimumHeight(64);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(icon);
    layout->addWidget(description);
    layout->addWidget(m_passwordEdit);
    layout->addStretch();
    layout->addWidget(m_finishButton);

    connect(m_finishButton, &QAbstractButton::clicked, this, &VaultPasswordPage::quitPasswordPage);
}

VaultPasswordPage::~VaultPasswordPage()
{
    m_passwordEdit->clear();
}

void VaultPasswordPage::showPassword()
{
    DSecureString password = Singleton<SecretManager>::instance()->lookupVaultPassword();
    if (!password.isEmpty()) {
        m_passwordEdit->setText(password);
        Singleton<SecretManager>::instance()->clearVaultPassword();
    }
}

void VaultPasswordPage::quitPasswordPage()
{
    m_passwordEdit->clear();
    emit requestRedirect(VaultController::makeVaultUrl());
}

// ----------------------------------------------

DFMVaultRecoveryKeyPages::DFMVaultRecoveryKeyPages(QWidget *parent)
    : DFMVaultPages(parent)
{
    VaultAskCreateKeyPage * askPage = new VaultAskCreateKeyPage(this);
    VaultVerifyUserPage * verifyPage = new VaultVerifyUserPage(this);
    VaultGeneratedKeyPage * generatedKeyPage = new VaultGeneratedKeyPage(this);
    VaultVerifyRecoveryKeyPage * enterRecoveryKeyPage = new VaultVerifyRecoveryKeyPage(this);
    VaultPasswordPage * vaultPasswordPage = new VaultPasswordPage(this);

    connect(askPage, &VaultAskCreateKeyPage::requestRedirect, this, &DFMVaultRecoveryKeyPages::requestRedirect);
    connect(verifyPage, &VaultVerifyUserPage::requestRedirect, this, &DFMVaultRecoveryKeyPages::requestRedirect);
    connect(generatedKeyPage, &VaultGeneratedKeyPage::requestRedirect, this, &DFMVaultRecoveryKeyPages::requestRedirect);
    connect(enterRecoveryKeyPage, &VaultVerifyRecoveryKeyPage::requestRedirect, this, &DFMVaultRecoveryKeyPages::requestRedirect);
    connect(vaultPasswordPage, &VaultPasswordPage::requestRedirect, this, &DFMVaultRecoveryKeyPages::requestRedirect);

    insertPage("ask", askPage);
    insertPage("verify", verifyPage);
    insertPage("generated_key", generatedKeyPage);
    insertPage("retrieve_password", enterRecoveryKeyPage);
    insertPage("password", vaultPasswordPage);

    connect(this, &DFMVaultPages::rootPageChanged, this, &DFMVaultRecoveryKeyPages::onRootPageChanged);
    connect(this, &DFMVaultRecoveryKeyPages::requestCreateRecoveryKey, generatedKeyPage, &VaultGeneratedKeyPage::startKeyGeneration);
    connect(this, &DFMVaultRecoveryKeyPages::requestPrepareRetrievePasswordPage, enterRecoveryKeyPage, &VaultVerifyRecoveryKeyPage::preparePage);
    connect(this, &DFMVaultRecoveryKeyPages::requestShowPassword, vaultPasswordPage, &VaultPasswordPage::showPassword);
}

DFMVaultRecoveryKeyPages::~DFMVaultRecoveryKeyPages()
{

}

QPair<DUrl, bool> DFMVaultRecoveryKeyPages::requireRedirect(VaultController::VaultState state)
{
    switch (state) {
    case VaultController::NotExisted:
        return {VaultController::makeVaultUrl("/", "setup"), true};
    default:
        break;
    }

    return DFMVaultPages::requireRedirect(state);
}

QString DFMVaultRecoveryKeyPages::pageString(const DUrl &url)
{
    // ask -> verify -> generated_key
    // recover_password -> password
    if (url.path() == "/ask") {
        return "ask";
    }
    if (url.path() == "/verify") {
        return "verify";
    }
    if (url.path() == "/generated_key") {
        return "generated_key";
    }
    if (url.path() == "/retrieve_password") {
        return "retrieve_password";
    }
    if (url.path() == "/password") {
        return "password";
    }
    return "verify";
}

void DFMVaultRecoveryKeyPages::onRootPageChanged(QString pageStr)
{
    if (pageStr == "generated_key") {
        if (!Singleton<SecretManager>::instance()->lookupVaultPassword().isEmpty()) {
            emit requestCreateRecoveryKey();
        }
    } else if (pageStr == "retrieve_password") {
        emit requestPrepareRetrievePasswordPage();
    } else if (pageStr == "password") {
        emit requestShowPassword();
    }
}

DFM_END_NAMESPACE
