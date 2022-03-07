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

#define TOOLTIP_SHOW_DURATION 3000

#include "vaultunlockpages.h"
#include "vaultretrievepassword.h"
#include "vaultretrievepassword.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaultglobaldefine.h"
#include "utils/vaulthelper.h"
#include "services/filemanager/vault/vaultservice.h"
#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/application/settings.h"

#include <DPushButton>
#include <DPasswordEdit>
#include <DFloatingWidget>
#include <DToolTip>
#include <DLabel>
#include <DFontSizeManager>

#include <QDir>
#include <QStandardPaths>
#include <QShowEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QToolTip>
#include <QEvent>
#include <QTimer>
#include <QProcess>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPVAULT_USE_NAMESPACE

VaultUnlockPages::VaultUnlockPages(QWidget *parent)
    : VaultPageBase(parent)
{
    tooltipTimer = new QTimer(this);
    connect(tooltipTimer, &QTimer::timeout,
            this, &VaultUnlockPages::slotTooltipTimerTimeout);

    setIcon(QIcon::fromTheme("dfm_vault"));
    setMinimumSize(396, 244);

    //! 标题
    DLabel *pTitle = new DLabel(tr("Unlock File Vault"), this);

    pTitle->setAlignment(Qt::AlignHCenter);
    pTitle->setMargin(0);
    //! Set font.
    DFontSizeManager::instance()->bind(pTitle, DFontSizeManager::T7, QFont::Medium);
    pTitle->setForegroundRole(DPalette::TextTitle);

    forgetPassword = new DLabel(tr("Forgot password?"));
    QFont font = pTitle->font();
    font.setPixelSize(12);
    forgetPassword->setFont(font);
    forgetPassword->installEventFilter(this);
    forgetPassword->setForegroundRole(DPalette::ColorType::LightLively);

    //! 密码编辑框
    passwordEdit = new DPasswordEdit(this);
    passwordEdit->lineEdit()->setPlaceholderText(tr("Password"));
    passwordEdit->lineEdit()->installEventFilter(this);
    passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    // 提示按钮
    tipsButton = new QPushButton(this);
    tipsButton->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));

    //! 找回密码页面
    retrievePage = new VaultRetrievePassword();

    //! 主视图
    QFrame *mainFrame = new QFrame(this);

    //! 布局
    QHBoxLayout *play1 = new QHBoxLayout();
    play1->setMargin(0);
    play1->addWidget(passwordEdit);
    play1->addWidget(tipsButton);

    QHBoxLayout *play2 = new QHBoxLayout();
    play2->setMargin(0);
    play2->addStretch(1);
    play2->addWidget(forgetPassword);
    forgetPassword->setAlignment(Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setMargin(0);
    mainLayout->addWidget(pTitle);
    mainLayout->addStretch();
    mainLayout->addLayout(play1);
    mainLayout->addLayout(play2);
    mainLayout->addStretch();

    mainFrame->setLayout(mainLayout);
    addContent(mainFrame);
    setSpacing(0);
    //! 防止点击按钮后界面隐藏
    setOnButtonClickedClose(false);

    QStringList btnList({ tr("Cancel", "button"), tr("Unlock", "button") });
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);
    getButton(1)->setEnabled(false);

    connect(this, &VaultUnlockPages::buttonClicked, this, &VaultUnlockPages::onButtonClicked);
    connect(passwordEdit, &DPasswordEdit::textChanged, this, &VaultUnlockPages::onPasswordChanged);
    connect(VaultHelper::vaultServiceInstance(), &VaultService::signalUnlockVaultState, this, &VaultUnlockPages::onVaultUlocked);
    connect(tipsButton, &QPushButton::clicked, this, [this] {
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
            QString hint = tr("Password hint: %1").arg(strPwdHint);
            showToolTip(hint, TOOLTIP_SHOW_DURATION, EN_ToolTip::Information);
        }
    });

    connect(static_cast<VaultRetrievePassword *>(retrievePage), &VaultRetrievePassword::signalReturn, this, &VaultUnlockPages::onReturnUnlockedPage);
}

void VaultUnlockPages::showEvent(QShowEvent *event)
{
    if (extraLockVault) {
        extraLockVault = false;
    }
    //! 重置所有控件状态
    passwordEdit->lineEdit()->clear();
    QLineEdit edit;
    QPalette palette = edit.palette();
    passwordEdit->lineEdit()->setPalette(palette);
    passwordEdit->setEchoMode(QLineEdit::Password);
    unlockByPwd = false;

    //! 如果密码提示信息为空，则隐藏提示按钮
    QString strPwdHint("");
    if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
        if (strPwdHint.isEmpty()) {
            tipsButton->hide();
        } else {
            tipsButton->show();
        }
    }

    VaultPageBase::showEvent(event);
}

void VaultUnlockPages::closeEvent(QCloseEvent *event)
{
    extraLockVault = true;

    VaultPageBase::closeEvent(event);
}

void VaultUnlockPages::showToolTip(const QString &text, int duration, VaultUnlockPages::EN_ToolTip enType)
{
    if (!tooltip) {
        tooltip = new DToolTip(text);
        tooltip->setObjectName("AlertTooltip");
        tooltip->setWordWrap(true);

        floatWidget = new DFloatingWidget;
        floatWidget->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        floatWidget->setStyleSheet("background-color: rgba(247, 247, 247, 0.6);");
        floatWidget->setWidget(tooltip);
    }
    if (EN_ToolTip::Warning == enType)
        tooltip->setForegroundRole(DPalette::TextWarning);
    else
        tooltip->setForegroundRole(DPalette::TextTitle);

    floatWidget->setParent(this);

    tooltip->setText(text);
    if (floatWidget->parent()) {
        floatWidget->setGeometry(8, 154, 68, 26);
        floatWidget->show();
        floatWidget->adjustSize();
        floatWidget->raise();
    }

    if (duration < 0) {
        return;
    }

    //! 重新启动定时器，定时隐藏tooltip
    if (tooltipTimer) {
        if (tooltipTimer->isActive())
            tooltipTimer->stop();
        tooltipTimer->start(duration);
    }
}

void VaultUnlockPages::hideEvent(QHideEvent *event)
{
    if (floatWidget)
        floatWidget->close();
    VaultPageBase::hideEvent(event);
}

void VaultUnlockPages::onButtonClicked(const int &index)
{
    if (index == 1) {
        //! 点击解锁后,灰化解锁按钮
        getButton(1)->setEnabled(false);

        QString strPwd = passwordEdit->text();

        QString strCipher("");
        if (InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
            unlockByPwd = true;
            VaultHelper::unlockVault(strCipher);
        } else {
            //! 设置密码输入框颜色
            //! 修复bug-51508 激活密码框警告状态
            passwordEdit->setAlert(true);
        }
        return;
    }

    close();
}

void VaultUnlockPages::onPasswordChanged(const QString &pwd)
{
    if (!pwd.isEmpty()) {
        getButton(1)->setEnabled(true);
        //! 修复bug-51508 取消密码框警告状态
        passwordEdit->setAlert(false);
    } else {
        getButton(1)->setEnabled(false);
    }
}

void VaultUnlockPages::onVaultUlocked(int state)
{
    if (unlockByPwd) {
        if (state == 0) {
            UrlRoute::regScheme(VaultHelper::scheme(), VaultHelper::rootUrl().path(), VaultHelper::icon(), false, tr("My Vault"));
            VaultHelper::defaultCdAction(VaultHelper::rootUrl());
            Settings setting(kVaultTimeConfigFile);
            setting.setValue(QString("VaultTime"), QString("InterviewTime"), QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            close();
        } else if (state == 1) {   //! cryfs没有成功卸载挂载目录
            //! 解决sp3bug-38885:注销系统时，cryfs卸载挂载目录会概率性失败
            //! 卸载挂载目录
            QProcess process;
            QString fusermountBinary = QStandardPaths::findExecutable("fusermount");
            process.start(fusermountBinary, { "-zu", QString(kVaultBasePath) + QDir::separator() + QString(kVaultDecryptDirName) });
            process.waitForStarted();
            process.waitForFinished();
            process.terminate();
            if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
                QString strPwd = passwordEdit->text();
                QString strCipher("");
                //! 判断密码是否正确
                if (InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
                    //                    VaultController::ins()->unlockVault(strCipher);
                    return;
                } else {   //! 密码不正确
                    //! 设置密码输入框颜色,并弹出tooltip
                    passwordEdit->lineEdit()->setStyleSheet("background-color:rgba(241, 57, 50, 0.15)");
                    passwordEdit->showAlertMessage(tr("Wrong password"));
                }
            }
        } else {
            //! error tips
            QString errMsg = tr("Failed to unlock file vault");
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.setTitle(errMsg);
            dialog.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
            dialog.exec();
        }

        unlockByPwd = false;
    }
}

void VaultUnlockPages::onReturnUnlockedPage()
{
    retrievePage->hide();
    this->show();
}

bool VaultUnlockPages::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj == forgetPassword) {
        if (evt->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(evt);
            if (mouseEvent->button() == Qt::LeftButton) {
                this->hide();
                retrievePage->show();
            }
        }
    }
    return DDialog::eventFilter(obj, evt);
}

void VaultUnlockPages::slotTooltipTimerTimeout()
{
    floatWidget->close();
}
