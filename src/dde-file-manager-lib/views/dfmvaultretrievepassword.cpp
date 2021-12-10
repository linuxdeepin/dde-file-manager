/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#include "dfmvaultretrievepassword.h"
#include "accessibility/ac-lib-file-manager.h"
#include "vault/operatorcenter.h"
#include "durl.h"
#include "controllers/vaultcontroller.h"
#include "dfmvaultunlockpages.h"

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

using namespace PolkitQt1;

DWIDGET_USE_NAMESPACE

const QString defaultKeyPath = VAULT_BASE_PATH + QString("/") + RSA_PUB_KEY_FILE_NAME + QString(".key");
const QString PolicyKitRetrievePasswordActionId = "com.deepin.filemanager.vault.VerifyKey.RetrievePassword";

DFMVaultRetrievePassword *DFMVaultRetrievePassword::instance()
{
    static DFMVaultRetrievePassword s_instance;
    return &s_instance;
}

void DFMVaultRetrievePassword::verificationKey()
{
    QString password;
    QString keyPath;
    switch (m_savePathTypeComboBox->currentIndex()) {
    case 0: {
        if (QFile::exists(defaultKeyPath)) {
            m_defaultFilePathEdit->setText(QString(DFMVAULT_ROOT) + RSA_PUB_KEY_FILE_NAME + QString(".key"));
            getButton(1)->setEnabled(true);
            keyPath = defaultKeyPath;
        } else {
            m_defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
            m_defaultFilePathEdit->setText("");
            getButton(1)->setEnabled(false);
        }
        break;
    }
    case 1:
        keyPath = m_filePathEdit->text();
        if (!QFile::exists(keyPath)) {
            m_filePathEdit->lineEdit()->setPlaceholderText(tr("Unable to get the key file"));
            m_filePathEdit->setText("");
            getButton(1)->setEnabled(false);
        } else {
            getButton(1)->setEnabled(true);
        }
        break;
    }

    if (OperatorCenter::getInstance()->verificationRetrievePassword(keyPath, password)) {
        setResultsPage(password);
    } else {
        m_verificationPrompt->setText(tr("Verification failed"));
    }
}

QString DFMVaultRetrievePassword::getUserName()
{
    QString userPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString userName = userPath.section("/", -1, -1);
    return userName;
}

void DFMVaultRetrievePassword::onButtonClicked(int index, const QString &text)
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
            // 用户权限认证(异步授权)
            auto ins = Authority::instance();
            ins->checkAuthorization(PolicyKitRetrievePasswordActionId,
                                    UnixProcessSubject(getpid()),
                                    Authority::AllowUserInteraction);
            connect(ins, &Authority::checkAuthorizationFinished,
                    this, &DFMVaultRetrievePassword::slotCheckAuthorizationFinished);
        }
        break;
    }
}

void DFMVaultRetrievePassword::onComboBoxIndex(int index)
{
    switch (index) {
    case 0: {
        m_defaultFilePathEdit->show();
        m_filePathEdit->hide();
        if (QFile::exists(defaultKeyPath)) {
            m_defaultFilePathEdit->setText(QString(DFMVAULT_ROOT) + RSA_PUB_KEY_FILE_NAME + QString(".key"));
            getButton(1)->setEnabled(true);
        } else {
            m_defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
            m_defaultFilePathEdit->setText("");
            getButton(1)->setEnabled(false);
        }
        m_verificationPrompt->setText("");
    } break;
    case 1:
        m_defaultFilePathEdit->hide();
        m_filePathEdit->show();
        if (QFile::exists(m_filePathEdit->text()))
            getButton(1)->setEnabled(true);
        else if (!m_filePathEdit->text().isEmpty() && m_filePathEdit->lineEdit()->placeholderText() != QString(tr("Unable to get the key file"))) {
            m_filePathEdit->lineEdit()->setPlaceholderText(tr("Unable to get the key file"));
            m_filePathEdit->setText("");
            getButton(1)->setEnabled(false);
        } else {
            m_filePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
            getButton(1)->setEnabled(false);
        }
        m_verificationPrompt->setText("");
        break;
    }
}

void DFMVaultRetrievePassword::onBtnSelectFilePath(const QString &path)
{
    m_filePathEdit->setText(path);
    if (!path.isEmpty())
        getButton(1)->setEnabled(true);
}

void DFMVaultRetrievePassword::slotCheckAuthorizationFinished(PolkitQt1::Authority::Result result)
{
    disconnect(Authority::instance(), &Authority::checkAuthorizationFinished,
               this, &DFMVaultRetrievePassword::slotCheckAuthorizationFinished);
    if (isVisible()) {
        if (result == Authority::Yes) {
            verificationKey();
        }
    }
}

DFMVaultRetrievePassword::DFMVaultRetrievePassword(QWidget *parent)
    : DFMVaultPageBase(parent)
{
    setIcon(QIcon::fromTheme("dfm_vault"));
    setFixedWidth(396);
    setMinimumHeight(270);

    setTitle(tr("Retrieve Password"));
    QLabel *title = this->findChild<QLabel *>("TitleLabel");
    if (title)
        DFontSizeManager::instance()->bind(title, DFontSizeManager::T7, QFont::Medium);

    m_savePathTypeComboBox = new QComboBox(this);
    AC_SET_ACCESSIBLE_NAME(m_savePathTypeComboBox, AC_VAULT_SAVE_PATH_TYPE_COMBOBOX);
    m_savePathTypeComboBox->addItem(tr("By key in the default path"));
    m_savePathTypeComboBox->addItem(tr("By key in the specified path"));

    m_filePathEdit = new DFileChooserEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_filePathEdit, AC_VAULT_SAVE_PUBKEY_FILE_EDIT);
    m_filePathEdit->lineEdit()->setPlaceholderText(tr("Select a path"));
    QFileDialog *fileDialog = new QFileDialog(this, QDir::homePath());
    fileDialog->setDirectoryUrl(QDir::homePath());
    fileDialog->setNameFilter(QString("KEY file(*.key)"));
    m_filePathEdit->setFileDialog(fileDialog);
    m_filePathEdit->lineEdit()->setReadOnly(true);
    m_filePathEdit->hide();

    m_defaultFilePathEdit = new QLineEdit(this);
    m_defaultFilePathEdit->setReadOnly(true);

    m_verificationPrompt = new DLabel(this);
    m_verificationPrompt->setForegroundRole(DPalette::TextWarning);
    m_verificationPrompt->setAlignment(Qt::AlignHCenter);
    DFontSizeManager::instance()->bind(m_verificationPrompt, DFontSizeManager::T7, QFont::Medium);

    m_PasswordRecoveryPage = new QFrame(this);

    m_passwordMsg = new DLabel(m_PasswordRecoveryPage);
    m_passwordMsg->setAlignment(Qt::AlignCenter);

    m_hintMsg = new DLabel(m_PasswordRecoveryPage);
    m_hintMsg->setAlignment(Qt::AlignCenter);
    m_hintMsg->setText(tr("Keep it safe"));

    QVBoxLayout *vlayout = new QVBoxLayout(m_PasswordRecoveryPage);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setAlignment(Qt::AlignTop);
    vlayout->addWidget(m_passwordMsg);
    vlayout->addWidget(m_hintMsg);
    vlayout->addStretch(1);
    m_PasswordRecoveryPage->hide();

    // 主视图
    m_selectKeyPage = new QFrame(this);

    // 布局
    QVBoxLayout *funLayout = new QVBoxLayout();
    funLayout->addWidget(m_savePathTypeComboBox);
    funLayout->addSpacing(4);
    funLayout->addWidget(m_filePathEdit);
    funLayout->addWidget(m_defaultFilePathEdit);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_selectKeyPage);
    mainLayout->setContentsMargins(25, 10, 25, 0);
    mainLayout->addStretch(1);
    mainLayout->addLayout(funLayout);
    mainLayout->addWidget(m_verificationPrompt);

    m_selectKeyPage->setLayout(mainLayout);
    addContent(m_selectKeyPage, Qt::AlignVCenter);

    // 防止点击按钮后界面隐藏
    setOnButtonClickedClose(false);

    btnList = QStringList({ tr("Back", "button"), tr("Verify Key", "button"), tr("Go to Unlock", "button"), tr("Close", "button") });
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);

    AC_SET_ACCESSIBLE_NAME(getButton(0), AC_VAULT_RETURN_UNLOCK_PAGE_BTN);
    AC_SET_ACCESSIBLE_NAME(getButton(1), AC_VAULT_VERIFY_PUKEY_FILE_BTN);

    connect(this, &DFMVaultRetrievePassword::buttonClicked, this, &DFMVaultRetrievePassword::onButtonClicked);

    connect(m_savePathTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndex(int)));

    connect(m_filePathEdit, &DFileChooserEdit::fileChoosed, this, &DFMVaultRetrievePassword::onBtnSelectFilePath);

    connect(VaultController::ins(), &VaultController::sigCloseWindow, this, &DFMVaultRetrievePassword::close);
}

void DFMVaultRetrievePassword::setVerificationPage()
{
    m_savePathTypeComboBox->setCurrentIndex(0);
    if (getContent(0) == m_PasswordRecoveryPage) {
        removeContent(m_PasswordRecoveryPage, false);
        setTitle(tr("Retrieve Password"));
        m_PasswordRecoveryPage->hide();
        addContent(m_selectKeyPage);
        m_selectKeyPage->show();
        m_filePathEdit->setText(QString(""));
    }
    m_verificationPrompt->setText("");
    setButtonText(0, btnList[0]);
    setButtonText(1, btnList[1]);
}

void DFMVaultRetrievePassword::setResultsPage(QString password)
{
    m_passwordMsg->setText(tr("Vault password: %1").arg(password));
    m_selectKeyPage->hide();

    setTitle(tr("Verification Successful"));
    removeContent(m_selectKeyPage, false);
    addContent(m_PasswordRecoveryPage);
    m_PasswordRecoveryPage->show();
    setButtonText(0, btnList[2]);
    setButtonText(1, btnList[3]);
    AC_SET_ACCESSIBLE_NAME(getButton(1), AC_VAULT_VERIFY_PUKEY_FILE_BTN);
}

void DFMVaultRetrievePassword::showEvent(QShowEvent *event)
{
    VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::RETRIEVEPASSWORDPAGE);
    if (QFile::exists(defaultKeyPath)) {
        m_defaultFilePathEdit->setText(QString(DFMVAULT_ROOT) + RSA_PUB_KEY_FILE_NAME + QString(".key"));
        getButton(1)->setEnabled(true);
    } else {
        m_defaultFilePathEdit->setPlaceholderText(tr("Unable to get the key file"));
        getButton(1)->setEnabled(false);
    }
    m_filePathEdit->setText("");
    setVerificationPage();

    //! 保持和解锁界面对齐
    QPoint pt = DFMVaultUnlockPages::instance()->pos();
    move(pt.x(), pt.y());

    event->accept();
}

void DFMVaultRetrievePassword::closeEvent(QCloseEvent *event)
{
    VaultController::ins()->setVauleCurrentPageMark(VaultPageMark::UNKNOWN);
    DFMVaultPageBase::closeEvent(event);
}
