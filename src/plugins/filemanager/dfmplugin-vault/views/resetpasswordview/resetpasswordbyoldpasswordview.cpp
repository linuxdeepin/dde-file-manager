// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resetpasswordbyoldpasswordview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaulthelper.h"
#include "dbus/vaultdbusutils.h"

#include <dfm-base/utils/dialogmanager.h>

#include <DFontSizeManager>
#include <DSpinner>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMouseEvent>
#include <QEvent>
#include <QApplication>
#include <QEventLoop>
#include <QtConcurrent>
#include <QFutureWatcher>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

constexpr int kPasswordLengthMax = 24;
constexpr int kToolTipShowDuration = 3000;

ResetPasswordByOldPasswordView::ResetPasswordByOldPasswordView(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

ResetPasswordByOldPasswordView::~ResetPasswordByOldPasswordView()
{
}

void ResetPasswordByOldPasswordView::initUI()
{
    DLabel *oldPasswordLabel = new DLabel(tr("Enter Old Password"), this);
    oldPasswordEdit = new DPasswordEdit(this);
    oldPasswordEdit->lineEdit()->setPlaceholderText(tr("Please enter old password"));
    oldPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    DLabel *newPasswordLabel = new DLabel(tr("Enter New Password"), this);
    newPasswordEdit = new DPasswordEdit(this);
    QRegularExpression regx("[A-Za-z0-9,.;?@/=()<>_+*&^%$#!`~'\"|]+");
    QValidator *validator = new QRegularExpressionValidator(regx, this);
    newPasswordEdit->lineEdit()->setValidator(validator);
    newPasswordEdit->lineEdit()->setPlaceholderText(tr("At least 8 characters, including A-Z, a-z, 0-9, and symbols"));
    newPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    DLabel *repeatPasswordLabel = new DLabel(tr("Repeat Password"), this);
    repeatPasswordEdit = new DPasswordEdit(this);
    repeatPasswordEdit->lineEdit()->setValidator(validator);
    repeatPasswordEdit->lineEdit()->setPlaceholderText(tr("Enter new password again"));
    repeatPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    DLabel *passwordHintLabel = new DLabel(tr("Password hint"), this);
    passwordHintEdit = new DLineEdit(this);
    passwordHintEdit->lineEdit()->setMaxLength(14);
    passwordHintEdit->setPlaceholderText(tr("Optional"));

    switchMethodLabel = new DLabel(tr("Use key verification"), this);
    DFontSizeManager::instance()->bind(switchMethodLabel, DFontSizeManager::T8, QFont::Medium);
    switchMethodLabel->setForegroundRole(DPalette::ColorType::LightLively);
    switchMethodLabel->installEventFilter(this);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    QHBoxLayout *oldPasswordLayout = new QHBoxLayout();
    oldPasswordLayout->setContentsMargins(0, 0, 0, 0);
    oldPasswordLayout->setSpacing(10);
    oldPasswordLabel->setFixedWidth(120);
    oldPasswordLayout->addWidget(oldPasswordLabel);
    oldPasswordLayout->addWidget(oldPasswordEdit);
    mainLayout->addLayout(oldPasswordLayout);

    QHBoxLayout *newPasswordLayout = new QHBoxLayout();
    newPasswordLayout->setContentsMargins(0, 0, 0, 0);
    newPasswordLayout->setSpacing(10);
    newPasswordLabel->setFixedWidth(120);
    newPasswordLayout->addWidget(newPasswordLabel);
    newPasswordLayout->addWidget(newPasswordEdit);
    mainLayout->addLayout(newPasswordLayout);

    QHBoxLayout *repeatPasswordLayout = new QHBoxLayout();
    repeatPasswordLayout->setContentsMargins(0, 0, 0, 0);
    repeatPasswordLayout->setSpacing(10);
    repeatPasswordLabel->setFixedWidth(120);
    repeatPasswordLayout->addWidget(repeatPasswordLabel);
    repeatPasswordLayout->addWidget(repeatPasswordEdit);
    mainLayout->addLayout(repeatPasswordLayout);

    QHBoxLayout *passwordHintLayout = new QHBoxLayout();
    passwordHintLayout->setContentsMargins(0, 0, 0, 0);
    passwordHintLayout->setSpacing(10);
    passwordHintLabel->setFixedWidth(120);
    passwordHintLayout->addWidget(passwordHintLabel);
    passwordHintLayout->addWidget(passwordHintEdit);
    mainLayout->addLayout(passwordHintLayout);

    mainLayout->addStretch();

    QHBoxLayout *switchLayout = new QHBoxLayout();
    switchLayout->setContentsMargins(0, 0, 0, 0);
    switchLayout->addStretch();
    switchLayout->addWidget(switchMethodLabel);
    mainLayout->addLayout(switchLayout);

    this->setLayout(mainLayout);

    // 加载动画（放在窗口中间，覆盖在内容上方）
    spinner = new DSpinner(this);
    spinner->setFixedSize(48, 48);
    spinner->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    spinner->setFocusPolicy(Qt::NoFocus);
    spinner->hide();

    // 初始化重置密码异步操作
    resetPasswordWatcher = new QFutureWatcher<ResetPasswordResult>(this);
    connect(resetPasswordWatcher, &QFutureWatcher<ResetPasswordResult>::finished, this, &ResetPasswordByOldPasswordView::onResetPasswordFinished);

    connect(oldPasswordEdit, &DPasswordEdit::textChanged, this, &ResetPasswordByOldPasswordView::onOldPasswordChanged);
    connect(newPasswordEdit, &DPasswordEdit::textChanged, this, &ResetPasswordByOldPasswordView::onNewPasswordChanged);
    connect(repeatPasswordEdit, &DPasswordEdit::textChanged, this, &ResetPasswordByOldPasswordView::onRepeatPasswordChanged);
}

QStringList ResetPasswordByOldPasswordView::btnText()
{
    return { tr("Cancel", "button"), tr("Reset Password", "button") };
}

QString ResetPasswordByOldPasswordView::titleText()
{
    return QString(tr("Reset Password"));
}

void ResetPasswordByOldPasswordView::buttonClicked(int index, const QString &text)
{
    if (index == 1) {
        emit sigBtnEnabled(1, false);

        // 验证新密码格式
        QString newPwd = newPasswordEdit->text();
        if (!checkPassword(newPwd)) {
            newPasswordEdit->setAlert(true);
            newPasswordEdit->showAlertMessage(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            emit sigBtnEnabled(0, true);
            return;
        }

        // 验证重复密码
        if (!checkRepeatPassword()) {
            repeatPasswordEdit->setAlert(true);
            repeatPasswordEdit->showAlertMessage(tr("Passwords do not match"), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            emit sigBtnEnabled(0, true);
            return;
        }

        // 检查错误输入次数限制
        int nLeftoverErrorTimes = VaultDBusUtils::getLeftoverErrorInputTimes();
        if (nLeftoverErrorTimes < 1) {
            int nNeedWaitMinutes = VaultDBusUtils::getNeedWaitMinutes();
            oldPasswordEdit->setAlert(true);
            oldPasswordEdit->showAlertMessage(tr("Please try again %1 minutes later").arg(nNeedWaitMinutes), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            emit sigBtnEnabled(0, true);
            return;
        }

        // 显示加载动画
        spinner->move((width() - spinner->width()) / 2, (height() - spinner->height()) / 2);
        spinner->show();
        spinner->raise();
        spinner->start();
        oldPasswordEdit->setEnabled(false);
        newPasswordEdit->setEnabled(false);
        repeatPasswordEdit->setEnabled(false);
        passwordHintEdit->setEnabled(false);
        // 重置过程不可取消，禁用Cancel按钮
        emit sigBtnEnabled(0, false);
        if (switchMethodLabel)
            switchMethodLabel->setEnabled(false);

        // 在子线程中执行重置密码操作
        QString oldPwd = oldPasswordEdit->text();
        QString passwordHint = passwordHintEdit ? passwordHintEdit->text() : QString();
        QFuture<ResetPasswordResult> future = QtConcurrent::run([oldPwd, newPwd, passwordHint]() -> ResetPasswordResult {
            ResetPasswordResult result;
            result.success = OperatorCenter::getInstance()->resetPasswordByOldPassword(oldPwd, newPwd, passwordHint);
            return result;
        });
        resetPasswordWatcher->setFuture(future);
    } else {
        emit sigCloseDialog();
    }
}

void ResetPasswordByOldPasswordView::onPasswordChanged()
{
    QString newPwd = newPasswordEdit->text();
    QString repeatPwd = repeatPasswordEdit->text();

    if (!newPwd.isEmpty() && !checkPassword(newPwd)) {
        newPasswordEdit->setAlert(true);
        newPasswordEdit->showAlertMessage(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"), kToolTipShowDuration);
    } else {
        newPasswordEdit->setAlert(false);
        newPasswordEdit->hideAlertMessage();
    }

    if (!repeatPwd.isEmpty() && !newPwd.isEmpty() && newPwd != repeatPwd) {
        repeatPasswordEdit->setAlert(true);
        repeatPasswordEdit->showAlertMessage(tr("Passwords do not match"), kToolTipShowDuration);
    } else {
        repeatPasswordEdit->setAlert(false);
        repeatPasswordEdit->hideAlertMessage();
    }

    if (checkInputInfo()) {
        emit sigBtnEnabled(1, true);
    } else {
        emit sigBtnEnabled(1, false);
    }
}

void ResetPasswordByOldPasswordView::onOldPasswordChanged(const QString &pwd)
{
    Q_UNUSED(pwd);
    onPasswordChanged();
    oldPasswordEdit->setAlert(false);
    oldPasswordEdit->hideAlertMessage();
}

void ResetPasswordByOldPasswordView::onNewPasswordChanged(const QString &pwd)
{
    Q_UNUSED(pwd);
    onPasswordChanged();
}

void ResetPasswordByOldPasswordView::onRepeatPasswordChanged(const QString &pwd)
{
    Q_UNUSED(pwd);
    onPasswordChanged();
}

bool ResetPasswordByOldPasswordView::checkPassword(const QString &password)
{
    QString strPassword = password;
    if (strPassword.length() > kPasswordLengthMax) {
        return false;
    }

    QRegularExpression rx("^(?![^a-z]+$)(?![^A-Z]+$)(?!\\D+$)(?![a-zA-Z0-9]+$).{8,}$");
    QRegularExpressionValidator v(rx);
    int pos = 0;
    QValidator::State res;
    res = v.validate(strPassword, pos);
    return QValidator::Acceptable == res;
}

bool ResetPasswordByOldPasswordView::checkRepeatPassword()
{
    const QString &strRepeatPassword = repeatPasswordEdit->text();
    const QString &strPassword = newPasswordEdit->text();
    return strRepeatPassword == strPassword;
}

bool ResetPasswordByOldPasswordView::checkInputInfo()
{
    return !oldPasswordEdit->text().isEmpty()
           && checkPassword(newPasswordEdit->text())
           && checkRepeatPassword();
}

void ResetPasswordByOldPasswordView::showEvent(QShowEvent *event)
{
    // 重置所有控件状态
    oldPasswordEdit->clear();
    oldPasswordEdit->setAlert(false);
    oldPasswordEdit->hideAlertMessage();
    oldPasswordEdit->setEnabled(true);
    newPasswordEdit->clear();
    newPasswordEdit->setAlert(false);
    newPasswordEdit->hideAlertMessage();
    newPasswordEdit->setEnabled(true);
    repeatPasswordEdit->clear();
    repeatPasswordEdit->setAlert(false);
    repeatPasswordEdit->hideAlertMessage();
    repeatPasswordEdit->setEnabled(true);
    if (passwordHintEdit) {
        passwordHintEdit->clear();
        passwordHintEdit->setPlaceholderText(tr("Optional"));
        passwordHintEdit->setEnabled(true);
    }
    spinner->stop();
    spinner->hide();
    if (resetPasswordWatcher && resetPasswordWatcher->isRunning()) {
        resetPasswordWatcher->cancel();
    }
    QFrame::showEvent(event);
}

bool ResetPasswordByOldPasswordView::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj == switchMethodLabel) {
        // 禁用状态下不响应切换，防止重置过程中切换页面导致崩溃
        if (!switchMethodLabel->isEnabled()) {
            return false;
        }
        if (evt->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(evt);
            if (mouseEvent->button() == Qt::LeftButton) {
                emit signalJump();
                return true;
            }
        }
    }
    return QFrame::eventFilter(obj, evt);
}

void ResetPasswordByOldPasswordView::onResetPasswordFinished()
{
    ResetPasswordResult result = resetPasswordWatcher->result();

    // 隐藏加载动画
    spinner->stop();
    spinner->hide();
    oldPasswordEdit->setEnabled(true);
    newPasswordEdit->setEnabled(true);
    repeatPasswordEdit->setEnabled(true);
    passwordHintEdit->setEnabled(true);
    // 重置结束后恢复Cancel按钮
    emit sigBtnEnabled(0, true);
    if (switchMethodLabel)
        switchMethodLabel->setEnabled(true);

    if (result.success) {
        // 密码重置成功，恢复错误次数限制
        VaultDBusUtils::restoreLeftoverErrorInputTimes();
        VaultDBusUtils::restoreNeedWaitMinutes();
        DialogManager::instance()->showMessageDialog(tr("Success"), tr("Password reset successfully"));
        emit sigCloseDialog();
    } else {
        // 重置失败
        oldPasswordEdit->setAlert(true);

        // 保险箱剩余错误密码输入次数减1
        VaultDBusUtils::leftoverErrorInputTimesMinusOne();

        int nLeftoverErrorTimes = VaultDBusUtils::getLeftoverErrorInputTimes();

        if (nLeftoverErrorTimes < 1) {
            // 计时10分钟后，恢复密码编辑框
            VaultDBusUtils::startTimerOfRestorePasswordInput();
            // 错误输入次数超过了限制
            int nNeedWaitMinutes = VaultDBusUtils::getNeedWaitMinutes();
            oldPasswordEdit->showAlertMessage(tr("Failed to reset password. Please try again %1 minutes later").arg(nNeedWaitMinutes), kToolTipShowDuration);
        } else {
            if (nLeftoverErrorTimes == 1)
                oldPasswordEdit->showAlertMessage(tr("Failed to reset password. One chance left"), kToolTipShowDuration);
            else
                oldPasswordEdit->showAlertMessage(tr("Failed to reset password. %1 chances left").arg(nLeftoverErrorTimes), kToolTipShowDuration);
        }

        emit sigBtnEnabled(1, true);
    }
}
