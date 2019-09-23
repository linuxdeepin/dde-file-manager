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

#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include <DFloatingButton>
#include <dpasswordedit.h>

DFM_BEGIN_NAMESPACE

VaultVerifyUserPage::VaultVerifyUserPage(QWidget *parent)
    : QWidget (parent)
{
    QLabel * description = new QLabel(tr("Enter the vault password"), this);
    description->setAlignment(Qt::AlignHCenter);

    m_unlockButton = new DFloatingButton(DStyle::SP_UnlockElement, this);
    m_passwordEdit = new DPasswordEdit(this);

    QPushButton * icon = new QPushButton(this);
    icon->setDisabled(true);
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_lock"));
    icon->setIconSize(QSize(64, 64));
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

    QPushButton * icon = new QPushButton(this);
    icon->setDisabled(true);
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_lock"));
    icon->setIconSize(QSize(64, 64));
    icon->setMinimumHeight(64);

    QLabel * title = new QLabel(tr("Find your recovery key below. Please take good care of it by printing, writing down or saving it to a USB flash drive"), this);
    QLabel * description = new QLabel(tr("Your recovery key is as important as your password. Please do not save the key file to this computer"), this);
    title->setAlignment(Qt::AlignHCenter);
    title->setWordWrap(true);
    description->setAlignment(Qt::AlignHCenter);
    description->setWordWrap(true);
    QFont font = title->font();
    font.setBold(true);
    title->setFont(font);

    m_generatedKeyEdit = new QPlainTextEdit(this);
    m_generatedKeyEdit->setPlainText(DSecureString("Hint text:\nAAAA-BBBB-CCCC-DDDD-EEEE-FFFF-GGGG-HHHH\n") +
                                     "Hint text:\nAAAA-BBBB-CCCC-DDDD-EEEE-FFFF-GGGG-HHHH");
    m_generatedKeyEdit->setMaximumHeight(100);
    m_generatedKeyEdit->setReadOnly(true);

    m_saveFileButton = new QPushButton(tr("Save to file"), this);
    m_finishButton = new QPushButton(tr("Finish"), this);

    layout->addStretch();
    layout->addWidget(icon);
    layout->addWidget(title);
    layout->addWidget(description);
    layout->addWidget(m_generatedKeyEdit);
    layout->addStretch();
    layout->addWidget(m_saveFileButton);
    layout->addWidget(m_finishButton);
}

VaultGeneratedKeyPage::~VaultGeneratedKeyPage()
{
    clearData();
}

void VaultGeneratedKeyPage::clearData()
{
    // TODO: remove saved password in GNOME keyring.
    m_generatedKeyEdit->clear();
}

// ----------------------------------------------

DFMVaultRecoveryKeyPages::DFMVaultRecoveryKeyPages(QWidget *parent)
    : DFMVaultPages(parent)
{
    VaultVerifyUserPage * verifyPage = new VaultVerifyUserPage(this);
    VaultGeneratedKeyPage * generatedKeyPage = new VaultGeneratedKeyPage(this);

    connect(verifyPage, &VaultVerifyUserPage::requestRedirect, this, &DFMVaultRecoveryKeyPages::requestRedirect);

    insertPage("verify", verifyPage);
    insertPage("generated_key", generatedKeyPage);
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
    // verify -> generated_key
    // recover_password -> password
    if (url.path() == "/verify") {
        return "verify";
    }
    if (url.path() == "/generated_key") {
        return "generated_key";
    }
    return "verify";
}

DFM_END_NAMESPACE
