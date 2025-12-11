// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "retrievepasswordview.h"
#include "views/radioframe.h"
#include "utils/vaultutils.h"
#include "utils/vaulthelper.h"
#include "utils/vaultautolock.h"
#include "utils/encryption/operatorcenter.h"

#include <dfm-framework/event/event.h>

#include <DFontSizeManager>
#include <DFileDialog>

#include <QStringList>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QDateTime>
#include <QLineEdit>
#include <QShowEvent>
#include <QGridLayout>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

constexpr char kVaultTRoot[] = "dfmvault:///";

const QString defaultKeyPath = kVaultBasePath + QString("/") + kRSAPUBKeyFileName + QString(".key");
const QString PolicyKitRetrievePasswordActionId = "com.deepin.filemanager.vault.VerifyKey.RetrievePassword";

RetrievePasswordView::RetrievePasswordView(QWidget *parent)
    : QFrame(parent)
{
    savePathTypeLabel = new DLabel(this);
    DFontSizeManager::instance()->bind(savePathTypeLabel, DFontSizeManager::T8, QFont::Medium);
    savePathTypeLabel->setForegroundRole(DPalette::ButtonText);
    savePathTypeLabel->setText(tr("Unlock the Safe via Key File"));

    filePathEdit = new DFileChooserEdit(this);
    filePathEdit->lineEdit()->setPlaceholderText(tr("Select Key File"));
    filePathEdit->setDirectoryUrl(QDir::homePath());
    filePathEdit->setFileMode(DFileDialog::ExistingFiles);
    filePathEdit->setNameFilters({ QString("KEY file(*.key)") });

    verificationPrompt = new DLabel(this);
    verificationPrompt->setForegroundRole(DPalette::TextWarning);
    verificationPrompt->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(verificationPrompt, DFontSizeManager::T7, QFont::Medium);

    RadioFrame *rdFrame = new RadioFrame;
    funLayout = new QGridLayout(rdFrame);
    rdFrame->setContentsMargins(10, 10, 10, 10);
    funLayout->addWidget(savePathTypeLabel, 0, 0, 1, 2);
    funLayout->addWidget(filePathEdit, 1, 0, 1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 10, 0, 0);
    mainLayout->addStretch(1);
    mainLayout->addWidget(rdFrame/*funLayout*/);
    mainLayout->addWidget(verificationPrompt);

    this->setLayout(mainLayout);
    connect(filePathEdit, &DFileChooserEdit::fileChoosed, this, &RetrievePasswordView::onBtnSelectFilePath);
    connect(filePathEdit->lineEdit(), &QLineEdit::textChanged, this, &RetrievePasswordView::onTextChanged);
#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(filePathEdit), AcName::kAcEditVaultRetrieveOtherPath);
    fmDebug() << "Vault: Testing accessibility tags added";
#endif
}

void RetrievePasswordView::setVerificationPage()
{
    filePathEdit->setText(QString(""));
    verificationPrompt->setText("");
}

void RetrievePasswordView::verificationKey()
{
    QString password;
    QString keyPath;
    keyPath = filePathEdit->text();
    if (!QFile::exists(keyPath)) {
        filePathEdit->lineEdit()->setPlaceholderText(tr("Unable to get the key file"));
        filePathEdit->setText("");
        emit sigBtnEnabled(1, false);
    } else {
        emit sigBtnEnabled(1, true);
    }

    if (OperatorCenter::getInstance()->verificationRetrievePassword(keyPath, password)) {
        validationResults = password;
        fmInfo() << "Vault: Key verification successful, password retrieved (length:" << password.length() << ")";
        if (VaultHelper::instance()->unlockVault(password)) {
            VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
            VaultAutoLock::instance()->slotUnlockVault(0);
            emit sigCloseDialog();
            VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(),
                                                     VaultHelper::instance()->rootUrl());
        } else {
            verificationPrompt->setText(tr("Unlock vault failed"));
        }
    } else {
        fmWarning() << "Vault: Key verification failed for path:" << keyPath;
        verificationPrompt->setText(tr("Verification failed"));
    }
}

QString RetrievePasswordView::getUserName()
{
    QString userPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString userName = userPath.section("/", -1, -1);
    return userName;
}

QStringList RetrievePasswordView::btnText()
{
    return { tr("Back", "button"), tr("Key Unlock", "button") };
}

QString RetrievePasswordView::titleText()
{
    return QString(tr("Key Unlocks Safe"));
}

void RetrievePasswordView::buttonClicked(int index, const QString &text)
{
    fmDebug() << "Vault: Retrieve password view button clicked - index:" << index << "text:" << text;
    switch (index) {
    case 0:
        fmDebug() << "Vault: Back button clicked, jumping to unlock page";
        emit signalJump(PageType::kUnlockPage);
        break;
    case 1:
        fmInfo() << "Vault: Verify Key button clicked, starting authorization";
        verificationKey();
        break;
    }
}

QString RetrievePasswordView::ValidationResults()
{
    return validationResults;
}

void RetrievePasswordView::onBtnSelectFilePath(const QString &path)
{
    filePathEdit->setText(path);
    if (!path.isEmpty())
        emit sigBtnEnabled(1, true);
}

void RetrievePasswordView::onTextChanged(const QString &path)
{
    if(!path.isEmpty())
        emit sigBtnEnabled(1, true);
    else
        emit sigBtnEnabled(1, false);
}

void RetrievePasswordView::showEvent(QShowEvent *event)
{
    setVerificationPage();

    QFrame::showEvent(event);
}
