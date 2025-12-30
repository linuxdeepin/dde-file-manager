// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resetpasswordbykeyfileview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/vaulthelper.h"

#include <dfm-base/utils/dialogmanager.h>

#include <DFontSizeManager>
#include <DSpinner>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMouseEvent>
#include <QEvent>
#include <QFile>
#include <QApplication>
#include <QEventLoop>
#include <QtConcurrent>
#include <QFutureWatcher>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

constexpr int kPasswordLengthMax = 24;
constexpr int kToolTipShowDuration = 3000;

ResetPasswordByKeyFileView::ResetPasswordByKeyFileView(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

ResetPasswordByKeyFileView::~ResetPasswordByKeyFileView()
{
}

void ResetPasswordByKeyFileView::initUI()
{
    DLabel *keyFileLabel = new DLabel(tr("Select Key File"), this);
    keyFileEdit = new DFileChooserEdit(this);
    keyFileEdit->lineEdit()->setPlaceholderText(tr("Select key file save path"));
    fileDialog = new DFileDialog(this, QDir::homePath());
    keyFileEdit->setDirectoryUrl(QDir::homePath());
    keyFileEdit->setFileMode(DFileDialog::ExistingFiles);
    keyFileEdit->setNameFilters({ QString("KEY file(*.key)") });
    keyFileEdit->setFileDialog(fileDialog);
    keyFileEdit->lineEdit()->setReadOnly(true);

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

    switchMethodLabel = new DLabel(tr("Use old password verification"), this);
    DFontSizeManager::instance()->bind(switchMethodLabel, DFontSizeManager::T8, QFont::Medium);
    switchMethodLabel->setForegroundRole(DPalette::ColorType::LightLively);
    switchMethodLabel->installEventFilter(this);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);

    QHBoxLayout *keyFileLayout = new QHBoxLayout();
    keyFileLayout->setContentsMargins(0, 0, 0, 0);
    keyFileLayout->setSpacing(10);
    keyFileLabel->setFixedWidth(120);
    keyFileLayout->addWidget(keyFileLabel);
    keyFileLayout->addWidget(keyFileEdit);
    mainLayout->addLayout(keyFileLayout);

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

    spinner = new DSpinner(this);
    spinner->setFixedSize(48, 48);
    spinner->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    spinner->setFocusPolicy(Qt::NoFocus);
    spinner->hide();

    resetPasswordWatcher = new QFutureWatcher<ResetPasswordResult>(this);
    connect(resetPasswordWatcher, &QFutureWatcher<ResetPasswordResult>::finished, this, &ResetPasswordByKeyFileView::onResetPasswordFinished);

    connect(keyFileEdit, &DFileChooserEdit::fileChoosed, this, &ResetPasswordByKeyFileView::onKeyFileSelected);
    connect(keyFileEdit, &DFileChooserEdit::dialogOpened, this, [this]() {
        fileDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    });
    connect(newPasswordEdit, &DPasswordEdit::textChanged, this, &ResetPasswordByKeyFileView::onNewPasswordChanged);
    connect(repeatPasswordEdit, &DPasswordEdit::textChanged, this, &ResetPasswordByKeyFileView::onRepeatPasswordChanged);
}

QStringList ResetPasswordByKeyFileView::btnText()
{
    return { tr("Cancel", "button"), tr("Reset Password", "button") };
}

QString ResetPasswordByKeyFileView::titleText()
{
    return QString(tr("Reset Password"));
}

void ResetPasswordByKeyFileView::buttonClicked(int index, const QString &text)
{
    if (index == 1) {
        emit sigBtnEnabled(1, false);

        // 验证密钥文件
        QString keyPath = keyFileEdit->text();
        if (keyPath.isEmpty() || !QFile::exists(keyPath)) {
            keyFileEdit->setAlert(true);
            keyFileEdit->showAlertMessage(tr("Unable to get the key file"), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            emit sigBtnEnabled(0, true);
            return;
        }

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

        // 重置密码只支持新版本保险箱
        if (!OperatorCenter::getInstance()->isNewVaultVersion()) {
            keyFileEdit->setAlert(true);
            keyFileEdit->showAlertMessage(tr("Cannot reset password for old version vault. Please upgrade the vault first."), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            emit sigBtnEnabled(0, true);
            return;
        }

        QFile keyFile(keyPath);
        if (!keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            keyFileEdit->setAlert(true);
            keyFileEdit->showAlertMessage(tr("Unable to read key file"), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            emit sigBtnEnabled(0, true);
            return;
        }
        QByteArray keyData = keyFile.readAll();
        keyFile.close();

        keyData = keyData.trimmed();
        QString recoveryKey = QString::fromUtf8(keyData).trimmed();

        if (recoveryKey.length() != 32) {
            keyFileEdit->setAlert(true);
            keyFileEdit->showAlertMessage(tr("Invalid recovery key format: expected 32 characters, got %1").arg(recoveryKey.length()), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            emit sigBtnEnabled(0, true);
            return;
        }

        QRegularExpression keyFormat("^[A-Za-z0-9]{32}$");
        QRegularExpressionMatch match = keyFormat.match(recoveryKey);
        if (!match.hasMatch()) {
            keyFileEdit->setAlert(true);
            keyFileEdit->showAlertMessage(tr("Invalid recovery key format: must contain only letters and numbers"), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            emit sigBtnEnabled(0, true);
            return;
        }

        spinner->move((width() - spinner->width()) / 2, (height() - spinner->height()) / 2);
        spinner->show();
        spinner->raise();
        spinner->start();
        keyFileEdit->setEnabled(false);
        newPasswordEdit->setEnabled(false);
        repeatPasswordEdit->setEnabled(false);
        passwordHintEdit->setEnabled(false);
        emit sigBtnEnabled(0, false);
        if (switchMethodLabel)
            switchMethodLabel->setEnabled(false);

        QString passwordHint = passwordHintEdit ? passwordHintEdit->text() : QString();
        QFuture<ResetPasswordResult> future = QtConcurrent::run([recoveryKey, newPwd, passwordHint]() -> ResetPasswordResult {
            ResetPasswordResult result;
            result.success = OperatorCenter::getInstance()->resetPasswordByRecoveryKey(recoveryKey, newPwd, passwordHint);
            return result;
        });
        resetPasswordWatcher->setFuture(future);
    } else {
        emit sigCloseDialog();
    }
}

void ResetPasswordByKeyFileView::onPasswordChanged()
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

void ResetPasswordByKeyFileView::onKeyFileSelected(const QString &path)
{
    Q_UNUSED(path);
    onPasswordChanged();
}

void ResetPasswordByKeyFileView::onNewPasswordChanged(const QString &pwd)
{
    Q_UNUSED(pwd);
    onPasswordChanged();
}

void ResetPasswordByKeyFileView::onRepeatPasswordChanged(const QString &pwd)
{
    Q_UNUSED(pwd);
    onPasswordChanged();
}

bool ResetPasswordByKeyFileView::checkPassword(const QString &password)
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

bool ResetPasswordByKeyFileView::checkRepeatPassword()
{
    const QString &strRepeatPassword = repeatPasswordEdit->text();
    const QString &strPassword = newPasswordEdit->text();
    return strRepeatPassword == strPassword;
}

bool ResetPasswordByKeyFileView::checkInputInfo()
{
    return !keyFileEdit->text().isEmpty()
           && checkPassword(newPasswordEdit->text())
           && checkRepeatPassword();
}

void ResetPasswordByKeyFileView::showEvent(QShowEvent *event)
{
    // 重置所有控件状态
    keyFileEdit->setText("");
    keyFileEdit->lineEdit()->setPlaceholderText(tr("Select key file save path"));
    keyFileEdit->setAlert(false);
    keyFileEdit->hideAlertMessage();
    keyFileEdit->setEnabled(true);
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

bool ResetPasswordByKeyFileView::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj == switchMethodLabel) {
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

void ResetPasswordByKeyFileView::onResetPasswordFinished()
{
    ResetPasswordResult result = resetPasswordWatcher->result();

    spinner->stop();
    spinner->hide();
    keyFileEdit->setEnabled(true);
    newPasswordEdit->setEnabled(true);
    repeatPasswordEdit->setEnabled(true);
    passwordHintEdit->setEnabled(true);
    emit sigBtnEnabled(0, true);
    if (switchMethodLabel)
        switchMethodLabel->setEnabled(true);

    if (result.success) {
        DialogManager::instance()->showMessageDialog(tr("Success"), tr("Password reset successfully"));
        emit sigCloseDialog();
    } else {
        keyFileEdit->setAlert(true);
        keyFileEdit->showAlertMessage(tr("Failed to reset password. Please check your key file."), kToolTipShowDuration);
        emit sigBtnEnabled(1, true);
    }
}
