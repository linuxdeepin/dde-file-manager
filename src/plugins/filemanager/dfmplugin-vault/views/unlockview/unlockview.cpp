// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "unlockview.h"
#include "utils/vaulthelper.h"
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
#include <DSpinner>

#include <QMouseEvent>
#include <QProcess>
#include <QDateTime>
#include <QStandardPaths>
#include <QApplication>
#include <QEventLoop>
#include <QtConcurrent>
#include <QFutureWatcher>

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
    forgetPassword = new DLabel(VaultHelper::instance()->getVaultVersion() ? tr("Forgot password?") : tr("Key unlock"));
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
    play2->setContentsMargins(0, 0, 0, 0);
    play2->addStretch(1);
    play2->addWidget(forgetPassword);
    forgetPassword->setAlignment(Qt::AlignRight);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addStretch();
    mainLayout->addLayout(play1);
    mainLayout->addLayout(play2);
    mainLayout->addStretch();

    this->setLayout(mainLayout);

    // 加载动画（放在窗口中间，覆盖在内容上方）
    spinner = new DSpinner(this);
    spinner->setFixedSize(48, 48);
    spinner->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    spinner->setFocusPolicy(Qt::NoFocus);
    spinner->hide();

    connect(passwordEdit, &DPasswordEdit::textChanged, this, &UnlockView::onPasswordChanged);
    connect(VaultHelper::instance(), &VaultHelper::sigUnlocked, this, &UnlockView::onVaultUlocked);
    connect(tipsButton, &QPushButton::clicked, this, [this] {
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
            QString hint = tr("Password hint: %1").arg(strPwdHint);
            showToolTip(hint, kToolTipShowDuration, ENToolTip::kInformation);
        }
    });

    tooltipTimer = new QTimer(this);
    connect(tooltipTimer, &QTimer::timeout, this, &UnlockView::slotTooltipTimerTimeout);

    // 初始化密码验证异步操作
    passwordCheckWatcher = new QFutureWatcher<PasswordCheckResult>(this);
    connect(passwordCheckWatcher, &QFutureWatcher<PasswordCheckResult>::finished, this, &UnlockView::onPasswordCheckFinished);

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(forgetPassword), AcName::kAcLabelVaultUnlockForget);
    AddATTag(qobject_cast<QWidget *>(passwordEdit), AcName::kAcEditVaultUnlockPassword);
    AddATTag(qobject_cast<QWidget *>(tipsButton), AcName::kAcBtnVaultUnlockHint);
#endif
}

void UnlockView::buttonClicked(int index, const QString &text)
{
    fmDebug() << "Vault: Unlock view button clicked - index:" << index << "text:" << text;
    if (index == 1) {
        emit sigBtnEnabled(1, false);

        if (!VaultHelper::instance()->enableUnlockVault()) {
            fmWarning() << "Vault: Cannot unlock vault under networking";
            showToolTip(tr("Can't unlock the vault under the networking!"), kToolTipShowDuration, ENToolTip::kInformation);
            emit sigBtnEnabled(1, true);
            return;
        }

        int nLeftoverErrorTimes = VaultDBusUtils::getLeftoverErrorInputTimes();

        if (nLeftoverErrorTimes < 1) {
            int nNeedWaitMinutes = VaultDBusUtils::getNeedWaitMinutes();
            fmWarning() << "Vault: Too many failed attempts, need to wait" << nNeedWaitMinutes << "minutes";
            passwordEdit->showAlertMessage(tr("Please try again %1 minutes later").arg(nNeedWaitMinutes));
            return;
        }

        QString strPwd = passwordEdit->text();
        pendingPassword = strPwd;

        // 显示加载动画
        spinner->move((width() - spinner->width()) / 2, (height() - spinner->height()) / 2);
        spinner->show();
        spinner->raise();
        spinner->start();
        passwordEdit->setEnabled(false);

        // 在子线程中执行密码验证并获取主密钥
        QFuture<PasswordCheckResult> future = QtConcurrent::run([strPwd]() -> PasswordCheckResult {
            PasswordCheckResult result;
            QString strCipher("");
            result.isValid = InterfaceActiveVault::checkPassword(strPwd, strCipher);
            result.masterKey = strCipher;   // checkPassword返回的是Base64编码的主密钥
            return result;
        });
        passwordCheckWatcher->setFuture(future);
        return;
    } else {
        emit sigCloseDialog();
    }
}

void UnlockView::onPasswordCheckFinished()
{
    PasswordCheckResult result = passwordCheckWatcher->result();
    QString strPwd = pendingPassword;
    pendingPassword.clear();

    // 隐藏加载动画
    spinner->stop();
    spinner->hide();
    passwordEdit->setEnabled(true);

    if (result.isValid) {
        // 直接使用已获取的主密钥（Base64编码），避免再次调用checkPassword
        unlockByPwd = true;

        // 保持 spinner 显示，等待解锁完成
        spinner->show();
        spinner->start();
        passwordEdit->setEnabled(false);
        // 在子线程中执行解锁操作（unlockVault内部会调用runVaultProcess，会阻塞）
        QtConcurrent::run([masterKey = result.masterKey]() {
            VaultHelper::instance()->unlockVault(masterKey);
        });
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
        int nLeftoverErrorTimes = VaultDBusUtils::getLeftoverErrorInputTimes();

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
        // 隐藏加载动画
        spinner->stop();
        spinner->hide();
        passwordEdit->setEnabled(true);

        if (state == static_cast<int>(ErrorCode::kSuccess)) {
            fmInfo() << "Vault: Vault unlocked successfully";
            VaultHelper::instance()->defaultCdAction(VaultHelper::instance()->currentWindowId(),
                                                     VaultHelper::instance()->rootUrl());
            VaultHelper::recordTime(kjsonGroupName, kjsonKeyInterviewItme);
            VaultAutoLock::instance()->slotUnlockVault(state);
            emit sigCloseDialog();
        } else if (state == static_cast<int>(ErrorCode::kUnspecifiedError)) {   //! cryfs没有成功卸载挂载目录
            fmWarning() << "Vault: Unspecified error occurred, attempting to unmount";
            //! cryfs卸载挂载目录会概率性失败
            //! 卸载挂载目录
            QProcess process;
            QString fusermountBinary = QStandardPaths::findExecutable("fusermount");
            process.start(fusermountBinary, { "-zu", QString(kVaultBasePath) + QDir::separator() + QString(kVaultDecryptDirName) });
            process.waitForStarted();
            process.waitForFinished();
            process.terminate();
            if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
                fmDebug() << "Vault: Fusermount completed successfully";
                QString strPwd = passwordEdit->text();
                QString strCipher("");
                //! 判断密码是否正确
                if (InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
                    return;
                } else {   //! 密码不正确
                    fmWarning() << "Vault: Password verification failed after unmount";
                    //! 设置密码输入框颜色,并弹出tooltip
                    passwordEdit->lineEdit()->setStyleSheet("background-color:rgba(241, 57, 50, 0.15)");
                    passwordEdit->showAlertMessage(tr("Wrong password"));
                }
            } else {
                fmCritical() << "Vault: Failed to unmount vault directory";
            }
        } else if (state == static_cast<int>(ErrorCode::kWrongPassword)) {
            fmWarning() << "Vault: Wrong password error during unlock";
            DDialog dialog(tr("Wrong password"), "", this);
            dialog.setIcon(QIcon::fromTheme("dde-file-manager"));
            dialog.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
            dialog.exec();
        } else {
            fmCritical() << "Vault: Failed to unlock file vault, error code:" << state;
            //! error tips
            QString errMsg = tr("Failed to unlock file vault, error code is %1").arg(state);
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dde-file-manager"));
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
    if (extraLockVault) {
        extraLockVault = false;
    }
    //! 重置所有控件状态
    passwordEdit->lineEdit()->clear();
    QLineEdit edit;
    QPalette palette = edit.palette();
    passwordEdit->lineEdit()->setPalette(palette);
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setEnabled(true);
    spinner->stop();
    spinner->hide();
    pendingPassword.clear();
    if (passwordCheckWatcher && passwordCheckWatcher->isRunning()) {
        passwordCheckWatcher->cancel();
    }
    unlockByPwd = false;

    //! 如果密码提示信息为空，则隐藏提示按钮
    QString strPwdHint("");
    if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
        if (strPwdHint.isEmpty()) {
            tipsButton->hide();
            fmDebug() << "Vault: Password hint is empty, hiding tips button";
        } else {
            tipsButton->show();
            fmDebug() << "Vault: Password hint available, showing tips button";
        }
    } else {
        fmWarning() << "Vault: Failed to get password hint";
    }
}

void UnlockView::closeEvent(QCloseEvent *event)
{
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
                if (VaultHelper::instance()->getVaultVersion())
                    emit signalJump(PageType::kRetrievePage);
                else
                    emit signalJump(PageType::kRecoverPage);
                return true;
            }
        }
    }
    return QFrame::eventFilter(obj, evt);
}
