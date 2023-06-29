// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unlockview.h"
#include "utils/vaulthelper.h"
#include "utils/policy/policymanager.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaultdefine.h"
#include "utils/vaultautolock.h"
#include "utils/servicemanager.h"
#include "utils/fileencrypthandle.h"
#include "dbus/vaultdbusutils.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/application/settings.h>

#include <dfm-framework/event/event.h>

#include <DFontSizeManager>
#include <DDialog>

#include <QMouseEvent>
#include <QProcess>
#include <QDateTime>
#include <QStandardPaths>

constexpr int kToolTipShowDuration = 3000;

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

UnlockView::UnlockView(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

UnlockView::~UnlockView()
{
}

QStringList UnlockView::btnText()
{
    return { tr("Cancel", "button"), tr("Unlock", "button") };
}

QString UnlockView::titleText()
{
    return QString(tr("Unlock File Vault"));
}

void UnlockView::initUI()
{
    //! Set font.
    forgetPassword = new DLabel(tr("Forgot password?"));
    DFontSizeManager::instance()->bind(forgetPassword, DFontSizeManager::T8, QFont::Medium);
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

    //! 布局
    QHBoxLayout *play1 = new QHBoxLayout();
    play1->setContentsMargins(0, 10, 0, 0);
    play1->addWidget(passwordEdit);
    play1->addWidget(tipsButton);

    QHBoxLayout *play2 = new QHBoxLayout();
    play2->setMargin(0);
    play2->addStretch(1);
    play2->addWidget(forgetPassword);
    forgetPassword->setAlignment(Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addStretch();
    mainLayout->addLayout(play1);
    mainLayout->addLayout(play2);
    mainLayout->addStretch();

    this->setLayout(mainLayout);

    connect(passwordEdit, &DPasswordEdit::textChanged, this, &UnlockView::onPasswordChanged);
    connect(FileEncryptHandle::instance(), &FileEncryptHandle::signalUnlockVault, this, &UnlockView::onVaultUlocked);
    connect(tipsButton, &QPushButton::clicked, this, [this] {
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
            QString hint = tr("Password hint: %1").arg(strPwdHint);
            showToolTip(hint, kToolTipShowDuration, ENToolTip::kInformation);
        }
    });

    tooltipTimer = new QTimer(this);
    connect(tooltipTimer, &QTimer::timeout, this, &UnlockView::slotTooltipTimerTimeout);

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(forgetPassword), AcName::kAcLabelVaultUnlockForget);
    AddATTag(qobject_cast<QWidget *>(passwordEdit), AcName::kAcEditVaultUnlockPassword);
    AddATTag(qobject_cast<QWidget *>(tipsButton), AcName::kAcBtnVaultUnlockHint);
#endif
}

void UnlockView::buttonClicked(int index, const QString &text)
{
    if (index == 1) {
        emit sigBtnEnabled(1, false);

        int nLeftoverErrorTimes = VaultDBusUtils::getLeftoverErrorInputTimes();

        if (nLeftoverErrorTimes < 1) {
            int nNeedWaitMinutes = VaultDBusUtils::getNeedWaitMinutes();
            passwordEdit->showAlertMessage(tr("Please try again %1 minutes later").arg(nNeedWaitMinutes));
            return;
        }

        QString strPwd = passwordEdit->text();

        QString strCipher("");
        if (InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
            unlockByPwd = true;
            VaultHelper::instance()->unlockVault(strCipher);
            // 密码输入正确后，剩余输入次数还原,需要等待的分钟数还原
            VaultDBusUtils::restoreLeftoverErrorInputTimes();
            VaultDBusUtils::restoreNeedWaitMinutes();
        } else {
            //! 设置密码输入框颜色
            //! 修复bug-51508 激活密码框警告状态
            passwordEdit->setAlert(true);

            // 保险箱剩余错误密码输入次数减1
            VaultDBusUtils::leftoverErrorInputTimesMinusOne();

            // 显示错误输入提示
            nLeftoverErrorTimes = VaultDBusUtils::getLeftoverErrorInputTimes();

            if (nLeftoverErrorTimes < 1) {
                // 计时10分钟后，恢复密码编辑框
                VaultDBusUtils::startTimerOfRestorePasswordInput();
                // 错误输入次数超过了限制
                int nNeedWaitMinutes = VaultDBusUtils::getNeedWaitMinutes();
                passwordEdit->showAlertMessage(tr("Wrong password, please try again %1 minutes later").arg(nNeedWaitMinutes));
            } else {
                if (nLeftoverErrorTimes == 1)
                    passwordEdit->showAlertMessage(tr("Wrong password, one chance left"));
                else
                    passwordEdit->showAlertMessage(tr("Wrong password, %1 chances left").arg(nLeftoverErrorTimes));
            }
        }
        return;
    } else {
        emit sigCloseDialog();
    }
}

void UnlockView::onPasswordChanged(const QString &pwd)
{
    if (!pwd.isEmpty()) {
        emit sigBtnEnabled(1, true);
        //! 修复bug-51508 取消密码框警告状态
        passwordEdit->setAlert(false);
    } else {
        emit sigBtnEnabled(1, false);
    }
}

void UnlockView::onVaultUlocked(int state)
{
    if (unlockByPwd) {
        if (state == 0) {
            VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(),
                                                     VaultHelper::instance()->rootUrl());
            VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
            VaultAutoLock::instance()->slotUnlockVault(state);
            emit sigCloseDialog();
        } else if (state == 1) {   //! cryfs没有成功卸载挂载目录
            //! cryfs卸载挂载目录会概率性失败
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
        } else if (state == static_cast<int>(ErrorCode::kWrongPassword)) {
            DDialog dialog(tr("Wrong password"), "", this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
            dialog.exec();
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

void UnlockView::slotTooltipTimerTimeout()
{
    floatWidget->close();
}

void UnlockView::showEvent(QShowEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kUnlockVaultPage);
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
}

void UnlockView::closeEvent(QCloseEvent *event)
{
    PolicyManager::setVauleCurrentPageMark(PolicyManager::VaultPageMark::kUnknown);
    extraLockVault = true;
    QFrame::closeEvent(event);
}

void UnlockView::showToolTip(const QString &text, int duration, UnlockView::ENToolTip enType)
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
    if (ENToolTip::kWarning == enType)
        tooltip->setForegroundRole(DPalette::TextWarning);
    else
        tooltip->setForegroundRole(DPalette::TextTitle);

    floatWidget->setParent(this);

    tooltip->setText(text);
    if (floatWidget->parent()) {
        floatWidget->setGeometry(0, 33, 68, 26);
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

void UnlockView::hideEvent(QHideEvent *event)
{
    if (floatWidget)
        floatWidget->close();
    QFrame::hideEvent(event);
}

bool UnlockView::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj == forgetPassword) {
        if (evt->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(evt);
            if (mouseEvent->button() == Qt::LeftButton) {
                emit signalJump(PageType::kRetrievePage);
            }
        }
    }
    return QFrame::eventFilter(obj, evt);
}
