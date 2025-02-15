// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "retrievepasswordview.h"
#include "utils/vaultutils.h"
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
    savePathTypeComboBox = new QComboBox(this);
    savePathTypeComboBox->addItem(tr("By key in the default path"));
    savePathTypeComboBox->addItem(tr("By key in the specified path"));

    filePathEdit = new DFileChooserEdit(this);
    filePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
    fileDialog = new DFileDialog(this, QDir::homePath());
    filePathEdit->setDirectoryUrl(QDir::homePath());
    filePathEdit->setFileMode(DFileDialog::ExistingFiles);
    filePathEdit->setNameFilters({ QString("KEY file(*.key)") });
    filePathEdit->setFileDialog(fileDialog);
    filePathEdit->lineEdit()->setReadOnly(true);
    filePathEdit->hide();

    defaultFilePathEdit = new QLineEdit(this);
    defaultFilePathEdit->setReadOnly(true);

    verificationPrompt = new DLabel(this);
    verificationPrompt->setForegroundRole(DPalette::TextWarning);
    verificationPrompt->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(verificationPrompt, DFontSizeManager::T7, QFont::Medium);

    //! 布局
    funLayout = new QGridLayout();
    funLayout->addWidget(savePathTypeComboBox, 0, 0, 1, 2);
    funLayout->addWidget(defaultFilePathEdit, 1, 0, 1, 2);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(25, 10, 25, 0);
    mainLayout->addStretch(1);
    mainLayout->addLayout(funLayout);
    mainLayout->addWidget(verificationPrompt);

    this->setLayout(mainLayout);

    connect(savePathTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndex(int)));

    connect(filePathEdit, &DFileChooserEdit::fileChoosed, this, &RetrievePasswordView::onBtnSelectFilePath);

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(savePathTypeComboBox), AcName::kAcComboVaultRetrieveMethod);
    AddATTag(qobject_cast<QWidget *>(defaultFilePathEdit), AcName::kAcEditVaultRetrieveDefaultPath);
    AddATTag(qobject_cast<QWidget *>(filePathEdit), AcName::kAcEditVaultRetrieveOtherPath);
#endif
}

void RetrievePasswordView::setVerificationPage()
{
    savePathTypeComboBox->setCurrentIndex(0);
    filePathEdit->setText(QString(""));
    verificationPrompt->setText("");
}

void RetrievePasswordView::verificationKey()
{
    QString password;
    QString keyPath;
    switch (savePathTypeComboBox->currentIndex()) {
    case 0: {
        if (QFile::exists(defaultKeyPath)) {
            defaultFilePathEdit->setText(QString(kVaultTRoot) + kRSAPUBKeyFileName + QString(".key"));
            emit sigBtnEnabled(1, true);
            keyPath = defaultKeyPath;
        } else {
            defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
            defaultFilePathEdit->setText("");
            emit sigBtnEnabled(1, false);
        }
        break;
    }
    case 1:
        keyPath = filePathEdit->text();
        if (!QFile::exists(keyPath)) {
            filePathEdit->lineEdit()->setPlaceholderText(tr("Unable to get the key file"));
            filePathEdit->setText("");
            emit sigBtnEnabled(1, false);
        } else {
            emit sigBtnEnabled(1, true);
        }
        break;
    }

    if (OperatorCenter::getInstance()->verificationRetrievePassword(keyPath, password)) {
        validationResults = password;
        emit signalJump(PageType::kPasswordRecoverPage);
    } else {
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
    return { tr("Back", "button"), tr("Verify Key", "button") };
}

QString RetrievePasswordView::titleText()
{
    return QString(tr("Retrieve Password"));
}

void RetrievePasswordView::buttonClicked(int index, const QString &text)
{
    switch (index) {
    case 0:
        emit signalJump(PageType::kUnlockPage);
        break;
    case 1:
        //! 用户权限认证(异步授权)
        VaultUtils::instance().showAuthorityDialog(kPolkitVaultRetrieve);
        connect(&VaultUtils::instance(), &VaultUtils::resultOfAuthority,
                this, &RetrievePasswordView::slotCheckAuthorizationFinished);
        break;
    }
}

QString RetrievePasswordView::ValidationResults()
{
    return validationResults;
}

void RetrievePasswordView::onComboBoxIndex(int index)
{
    switch (index) {
    case 0: {
        funLayout->removeWidget(filePathEdit);
        funLayout->addWidget(defaultFilePathEdit, 1, 0, 1, 2);
        defaultFilePathEdit->show();
        filePathEdit->hide();
        if (QFile::exists(defaultKeyPath)) {
            defaultFilePathEdit->setText(QString(kVaultTRoot) + kRSAPUBKeyFileName + QString(".key"));
            emit sigBtnEnabled(1, true);
        } else {
            defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
            defaultFilePathEdit->setText("");
            emit sigBtnEnabled(1, false);
        }
        verificationPrompt->setText("");
    } break;
    case 1:
        funLayout->removeWidget(defaultFilePathEdit);
        funLayout->addWidget(filePathEdit, 1, 0, 1, 2);
        defaultFilePathEdit->hide();
        filePathEdit->show();
        fileDialog->setWindowFlags(Qt::WindowStaysOnTopHint);
        if (QFile::exists(filePathEdit->text()))
            emit sigBtnEnabled(1, true);
        else if (!filePathEdit->text().isEmpty() && filePathEdit->lineEdit()->placeholderText() != QString(tr("Unable to get the key file"))) {
            filePathEdit->lineEdit()->setPlaceholderText(tr("Unable to get the key file"));
            filePathEdit->setText("");
            emit sigBtnEnabled(1, false);
        } else {
            filePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
            emit sigBtnEnabled(1, false);
        }
        verificationPrompt->setText("");
        break;
    }
}

void RetrievePasswordView::onBtnSelectFilePath(const QString &path)
{
    filePathEdit->setText(path);
    if (!path.isEmpty())
        emit sigBtnEnabled(1, true);
}

void RetrievePasswordView::slotCheckAuthorizationFinished(bool result)
{
    disconnect(&VaultUtils::instance(), &VaultUtils::resultOfAuthority,
               this, &RetrievePasswordView::slotCheckAuthorizationFinished);
    if (isVisible()) {
        if (result) {
            verificationKey();
        }
    }
}

void RetrievePasswordView::showEvent(QShowEvent *event)
{
    if (QFile::exists(defaultKeyPath)) {
        defaultFilePathEdit->setText(QString(kVaultTRoot) + kRSAPUBKeyFileName + QString(".key"));
        emit sigBtnEnabled(1, true);
    } else {
        defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
        emit sigBtnEnabled(1, false);
    }
    filePathEdit->setText("");
    setVerificationPage();

    QFrame::showEvent(event);
}
