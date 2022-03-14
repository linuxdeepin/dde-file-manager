/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#include "vaultretrievepassword.h"
#include "vaultunlockpages.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"

#include <DFontSizeManager>

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

using namespace PolkitQt1;

DWIDGET_USE_NAMESPACE
DPVAULT_USE_NAMESPACE

constexpr char kVaultTRoot[] = "dfmvault:///";

const QString defaultKeyPath = kVaultBasePath + QString("/") + kRSAPUBKeyFileName + QString(".key");
const QString PolicyKitRetrievePasswordActionId = "com.deepin.filemanager.vault.VerifyKey.RetrievePassword";

void VaultRetrievePassword::verificationKey()
{
    QString password;
    QString keyPath;
    switch (savePathTypeComboBox->currentIndex()) {
    case 0: {
        if (QFile::exists(defaultKeyPath)) {
            defaultFilePathEdit->setText(QString(kVaultTRoot) + kRSAPUBKeyFileName + QString(".key"));
            getButton(1)->setEnabled(true);
            keyPath = defaultKeyPath;
        } else {
            defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
            defaultFilePathEdit->setText("");
            getButton(1)->setEnabled(false);
        }
        break;
    }
    case 1:
        keyPath = filePathEdit->text();
        if (!QFile::exists(keyPath)) {
            filePathEdit->lineEdit()->setPlaceholderText(tr("Unable to get the key file"));
            filePathEdit->setText("");
            getButton(1)->setEnabled(false);
        } else {
            getButton(1)->setEnabled(true);
        }
        break;
    }

    if (OperatorCenter::getInstance()->verificationRetrievePassword(keyPath, password)) {
        setResultsPage(password);
    } else {
        verificationPrompt->setText(tr("Verification failed"));
    }
}

QString VaultRetrievePassword::getUserName()
{
    QString userPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString userName = userPath.section("/", -1, -1);
    return userName;
}

void VaultRetrievePassword::onButtonClicked(int index, const QString &text)
{
    switch (index) {
    case 0:
        emit signalReturn();
        break;
    case 1:
        if (text == btnList[3]) {
            setOnButtonClickedClose(true);   //! 按钮1关闭窗口能力打开
        } else {
            setOnButtonClickedClose(false);   //! 按钮1关闭窗口能力关闭
            //! 用户权限认证(异步授权)
            auto ins = Authority::instance();
            ins->checkAuthorization(PolicyKitRetrievePasswordActionId,
                                    UnixProcessSubject(getpid()),
                                    Authority::AllowUserInteraction);
            connect(ins, &Authority::checkAuthorizationFinished,
                    this, &VaultRetrievePassword::slotCheckAuthorizationFinished);
        }
        break;
    }
}

void VaultRetrievePassword::onComboBoxIndex(int index)
{
    switch (index) {
    case 0: {
        defaultFilePathEdit->show();
        filePathEdit->hide();
        if (QFile::exists(defaultKeyPath)) {
            defaultFilePathEdit->setText(QString(kVaultTRoot) + kRSAPUBKeyFileName + QString(".key"));
            getButton(1)->setEnabled(true);
        } else {
            defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
            defaultFilePathEdit->setText("");
            getButton(1)->setEnabled(false);
        }
        verificationPrompt->setText("");
    } break;
    case 1:
        defaultFilePathEdit->hide();
        filePathEdit->show();
        if (QFile::exists(filePathEdit->text()))
            getButton(1)->setEnabled(true);
        else if (!filePathEdit->text().isEmpty() && filePathEdit->lineEdit()->placeholderText() != QString(tr("Unable to get the key file"))) {
            filePathEdit->lineEdit()->setPlaceholderText(tr("Unable to get the key file"));
            filePathEdit->setText("");
            getButton(1)->setEnabled(false);
        } else {
            filePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
            getButton(1)->setEnabled(false);
        }
        verificationPrompt->setText("");
        break;
    }
}

void VaultRetrievePassword::onBtnSelectFilePath(const QString &path)
{
    filePathEdit->setText(path);
    if (!path.isEmpty())
        getButton(1)->setEnabled(true);
}

void VaultRetrievePassword::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &VaultRetrievePassword::slotCheckAuthorizationFinished);
    if (isVisible()) {
        if (result == Authority::Yes) {
            verificationKey();
        }
    }
}

VaultRetrievePassword::VaultRetrievePassword(QWidget *parent)
    : VaultPageBase(parent)
{
    setIcon(QIcon::fromTheme("dfm_vault"));
    setFixedWidth(396);
    setMinimumHeight(270);

    setTitle(tr("Retrieve Password"));
    QLabel *title = this->findChild<QLabel *>("TitleLabel");
    if (title)
        DFontSizeManager::instance()->bind(title, DFontSizeManager::T7, QFont::Medium);

    savePathTypeComboBox = new QComboBox(this);
    savePathTypeComboBox->addItem(tr("By key in the default path"));
    savePathTypeComboBox->addItem(tr("By key in the specified path"));

    filePathEdit = new DFileChooserEdit(this);
    filePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
    QFileDialog *fileDialog = new QFileDialog(this, QDir::homePath());
    fileDialog->setDirectoryUrl(QDir::homePath());
    fileDialog->setNameFilter(QString("KEY file(*.key)"));
    filePathEdit->setFileDialog(fileDialog);
    filePathEdit->lineEdit()->setReadOnly(true);
    filePathEdit->hide();

    defaultFilePathEdit = new QLineEdit(this);
    defaultFilePathEdit->setReadOnly(true);

    verificationPrompt = new DLabel(this);
    verificationPrompt->setForegroundRole(DPalette::TextWarning);
    verificationPrompt->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(verificationPrompt, DFontSizeManager::T7, QFont::Medium);

    passwordRecoveryPage = new QFrame(this);

    passwordMsg = new DLabel(passwordRecoveryPage);
    passwordMsg->setAlignment(Qt::AlignCenter);

    hintMsg = new DLabel(passwordRecoveryPage);
    hintMsg->setAlignment(Qt::AlignCenter);
    hintMsg->setText(tr("Keep it safe"));

    QVBoxLayout *vlayout = new QVBoxLayout(passwordRecoveryPage);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    vlayout->addWidget(passwordMsg);
    vlayout->addWidget(hintMsg);
    vlayout->addStretch(1);
    passwordRecoveryPage->hide();

    //! 主视图
    selectKeyPage = new QFrame(this);

    //! 布局
    QVBoxLayout *funLayout = new QVBoxLayout();
    funLayout->addWidget(savePathTypeComboBox);
    funLayout->addSpacing(4);
    funLayout->addWidget(filePathEdit);
    funLayout->addWidget(defaultFilePathEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout(selectKeyPage);
    mainLayout->setContentsMargins(25, 10, 25, 0);
    mainLayout->addStretch(1);
    mainLayout->addLayout(funLayout);
    mainLayout->addWidget(verificationPrompt);

    selectKeyPage->setLayout(mainLayout);
    addContent(selectKeyPage, Qt::AlignVCenter);

    //! 防止点击按钮后界面隐藏
    setOnButtonClickedClose(false);

    btnList = QStringList({ tr("Back", "button"), tr("Verify Key", "button"), tr("Go to Unlock", "button"), tr("Close", "button") });
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);

    connect(this, &VaultRetrievePassword::buttonClicked, this, &VaultRetrievePassword::onButtonClicked);

    connect(savePathTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndex(int)));

    connect(filePathEdit, &DFileChooserEdit::fileChoosed, this, &VaultRetrievePassword::onBtnSelectFilePath);
}

void VaultRetrievePassword::setVerificationPage()
{
    savePathTypeComboBox->setCurrentIndex(0);
    if (getContent(0) == passwordRecoveryPage) {
        removeContent(passwordRecoveryPage, false);
        setTitle(tr("Retrieve Password"));
        passwordRecoveryPage->hide();
        addContent(selectKeyPage);
        selectKeyPage->show();
        filePathEdit->setText(QString(""));
    }
    verificationPrompt->setText("");
    setButtonText(0, btnList[0]);
    setButtonText(1, btnList[1]);
}

void VaultRetrievePassword::setResultsPage(QString password)
{
    passwordMsg->setText(tr("Vault password: %1").arg(password));
    selectKeyPage->hide();

    setTitle(tr("Verification Successful"));
    removeContent(selectKeyPage, false);
    addContent(passwordRecoveryPage);
    passwordRecoveryPage->show();
    setButtonText(0, btnList[2]);
    setButtonText(1, btnList[3]);
}

void VaultRetrievePassword::showEvent(QShowEvent *event)
{
    VaultHelper::instance()->setVauleCurrentPageMark(VaultHelper::VaultPageMark::kRetrievePassWordPage);
    if (QFile::exists(defaultKeyPath)) {
        defaultFilePathEdit->setText(QString(kVaultTRoot) + kRSAPUBKeyFileName + QString(".key"));
        getButton(1)->setEnabled(true);
    } else {
        defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
        getButton(1)->setEnabled(false);
    }
    filePathEdit->setText("");
    setVerificationPage();

    VaultPageBase::showEvent(event);
}

void VaultRetrievePassword::closeEvent(QCloseEvent *event)
{
    VaultHelper::instance()->setVauleCurrentPageMark(VaultHelper::VaultPageMark::kUnknown);
    VaultPageBase::closeEvent(event);
}
