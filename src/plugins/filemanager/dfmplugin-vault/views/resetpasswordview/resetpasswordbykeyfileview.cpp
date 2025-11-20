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
#include <QRegExp>
#include <QRegExpValidator>
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
    QRegExp regx("[A-Za-z0-9,.;?@/=()<>_+*&^%$#!`~'\"|]+");
    QValidator *validator = new QRegExpValidator(regx, this);
    newPasswordEdit->lineEdit()->setValidator(validator);
    newPasswordEdit->lineEdit()->setPlaceholderText(tr("At least 8 characters, including A-Z, a-z, 0-9, and symbols"));
    newPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    DLabel *repeatPasswordLabel = new DLabel(tr("Repeat Password"), this);
    repeatPasswordEdit = new DPasswordEdit(this);
    repeatPasswordEdit->lineEdit()->setValidator(validator);
    repeatPasswordEdit->lineEdit()->setPlaceholderText(tr("Enter new password again"));
    repeatPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    switchMethodLabel = new DLabel(tr("Use old password verification"), this);
    DFontSizeManager::instance()->bind(switchMethodLabel, DFontSizeManager::T8, QFont::Medium);
    switchMethodLabel->setForegroundRole(DPalette::ColorType::LightLively);
    switchMethodLabel->installEventFilter(this);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(10);

    QHBoxLayout *keyFileLayout = new QHBoxLayout();
    keyFileLayout->setMargin(0);
    keyFileLayout->setSpacing(10);
    keyFileLabel->setFixedWidth(120);
    keyFileLayout->addWidget(keyFileLabel);
    keyFileLayout->addWidget(keyFileEdit);
    mainLayout->addLayout(keyFileLayout);

    QHBoxLayout *newPasswordLayout = new QHBoxLayout();
    newPasswordLayout->setMargin(0);
    newPasswordLayout->setSpacing(10);
    newPasswordLabel->setFixedWidth(120);
    newPasswordLayout->addWidget(newPasswordLabel);
    newPasswordLayout->addWidget(newPasswordEdit);
    mainLayout->addLayout(newPasswordLayout);

    QHBoxLayout *repeatPasswordLayout = new QHBoxLayout();
    repeatPasswordLayout->setMargin(0);
    repeatPasswordLayout->setSpacing(10);
    repeatPasswordLabel->setFixedWidth(120);
    repeatPasswordLayout->addWidget(repeatPasswordLabel);
    repeatPasswordLayout->addWidget(repeatPasswordEdit);
    mainLayout->addLayout(repeatPasswordLayout);

    mainLayout->addStretch();

    QHBoxLayout *switchLayout = new QHBoxLayout();
    switchLayout->setMargin(0);
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
            keyFileEdit->lineEdit()->setPlaceholderText(tr("Unable to get the key file"));
            keyFileEdit->setText("");
            emit sigBtnEnabled(1, true);
            return;
        }

        // 验证新密码格式
        QString newPwd = newPasswordEdit->text();
        if (!checkPassword(newPwd)) {
            newPasswordEdit->setAlert(true);
            newPasswordEdit->showAlertMessage(tr("≥ 8 chars, contains A-Z, a-z, 0-9, and symbols"), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            return;
        }

        // 验证重复密码
        if (!checkRepeatPassword()) {
            repeatPasswordEdit->setAlert(true);
            repeatPasswordEdit->showAlertMessage(tr("Passwords do not match"), kToolTipShowDuration);
            emit sigBtnEnabled(1, true);
            return;
        }

        // 从密钥文件获取恢复密钥
        QString recoveryKey;
        bool isNewVersion = OperatorCenter::getInstance()->isNewVaultVersion();

        if (isNewVersion) {
            QFile keyFile(keyPath);
            if (!keyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                keyFileEdit->lineEdit()->setPlaceholderText(tr("Unable to read key file"));
                keyFileEdit->setText("");
                emit sigBtnEnabled(1, true);
                return;
            }
            QByteArray keyData = keyFile.readAll();
            keyFile.close();

            // 去除所有空白字符（包括换行符、空格、制表符等）
            keyData = keyData.trimmed();
            recoveryKey = QString::fromUtf8(keyData).trimmed();

            // 验证恢复密钥格式（应该是32个字符，只包含字母和数字）
            if (recoveryKey.length() != 32) {
                keyFileEdit->lineEdit()->setPlaceholderText(tr("Invalid recovery key format: expected 32 characters, got %1").arg(recoveryKey.length()));
                keyFileEdit->setText("");
                emit sigBtnEnabled(1, true);
                return;
            }

            // 验证恢复密钥只包含字母和数字
            QRegExp keyFormat("^[A-Za-z0-9]{32}$");
            if (!keyFormat.exactMatch(recoveryKey)) {
                keyFileEdit->lineEdit()->setPlaceholderText(tr("Invalid recovery key format: must contain only letters and numbers"));
                keyFileEdit->setText("");
                emit sigBtnEnabled(1, true);
                return;
            }
        } else {
            // 旧版本：暂时不支持，显示错误
            keyFileEdit->lineEdit()->setPlaceholderText(tr("Old version migration not supported yet"));
            keyFileEdit->setText("");
            emit sigBtnEnabled(1, true);
            return;
        }

        // 显示加载动画
        spinner->move((width() - spinner->width()) / 2, (height() - spinner->height()) / 2);
        spinner->show();
        spinner->raise();
        spinner->start();
        keyFileEdit->setEnabled(false);
        newPasswordEdit->setEnabled(false);
        repeatPasswordEdit->setEnabled(false);

        // 在子线程中执行重置密码操作
        QFuture<ResetPasswordResult> future = QtConcurrent::run([recoveryKey, newPwd]() -> ResetPasswordResult {
            ResetPasswordResult result;
            result.success = OperatorCenter::getInstance()->resetPasswordByRecoveryKey(recoveryKey, newPwd);
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

    QRegExp rx("^(?![^a-z]+$)(?![^A-Z]+$)(?!\\D+$)(?![a-zA-Z0-9]+$).{8,}$");
    QRegExpValidator v(rx);
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
    keyFileEdit->setEnabled(true);
    newPasswordEdit->clear();
    newPasswordEdit->setAlert(false);
    newPasswordEdit->hideAlertMessage();
    newPasswordEdit->setEnabled(true);
    repeatPasswordEdit->clear();
    repeatPasswordEdit->setAlert(false);
    repeatPasswordEdit->hideAlertMessage();
    repeatPasswordEdit->setEnabled(true);
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

    // 隐藏加载动画
    spinner->stop();
    spinner->hide();
    keyFileEdit->setEnabled(true);
    newPasswordEdit->setEnabled(true);
    repeatPasswordEdit->setEnabled(true);

    if (result.success) {
        // 密码重置成功
        DialogManager::instance()->showMessageDialog(DialogManager::kMsgInfo, tr("Success"), tr("Password reset successfully"));
        emit sigCloseDialog();
    } else {
        // 重置失败
        keyFileEdit->lineEdit()->setPlaceholderText(tr("Failed to reset password. Please check your key file."));
        keyFileEdit->setText("");
        emit sigBtnEnabled(1, true);
    }
}
