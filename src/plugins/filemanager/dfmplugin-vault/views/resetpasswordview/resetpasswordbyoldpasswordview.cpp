// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "resetpasswordbyoldpasswordview.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/interfaceactivevault.h"
#include "utils/vaulthelper.h"

#include <DFontSizeManager>
#include <DDialog>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegExp>
#include <QRegExpValidator>
#include <QMouseEvent>
#include <QEvent>

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
    // 旧密码输入框
    DLabel *oldPasswordLabel = new DLabel(tr("Enter Old Password"), this);
    oldPasswordEdit = new DPasswordEdit(this);
    oldPasswordEdit->lineEdit()->setPlaceholderText(tr("Please enter old password"));
    oldPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    oldPasswordEdit->setEchoButtonIsVisible(false);

    // 新密码输入框
    DLabel *newPasswordLabel = new DLabel(tr("Enter New Password"), this);
    newPasswordEdit = new DPasswordEdit(this);
    QRegExp regx("[A-Za-z0-9,.;?@/=()<>_+*&^%$#!`~'\"|]+");
    QValidator *validator = new QRegExpValidator(regx, this);
    newPasswordEdit->lineEdit()->setValidator(validator);
    newPasswordEdit->lineEdit()->setPlaceholderText(tr("At least 8 characters, including A-Z, a-z, 0-9, and symbols"));
    newPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    newPasswordEdit->setEchoButtonIsVisible(false);

    // 重复密码输入框
    DLabel *repeatPasswordLabel = new DLabel(tr("Repeat Password"), this);
    repeatPasswordEdit = new DPasswordEdit(this);
    repeatPasswordEdit->lineEdit()->setValidator(validator);
    repeatPasswordEdit->lineEdit()->setPlaceholderText(tr("Enter new password again"));
    repeatPasswordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    repeatPasswordEdit->setEchoButtonIsVisible(false);

    // 切换方式标签
    switchMethodLabel = new DLabel(tr("Use key verification"), this);
    DFontSizeManager::instance()->bind(switchMethodLabel, DFontSizeManager::T8, QFont::Medium);
    switchMethodLabel->setForegroundRole(DPalette::ColorType::LightLively);
    switchMethodLabel->installEventFilter(this);

    // 布局
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->setSpacing(10);

    QHBoxLayout *oldPasswordLayout = new QHBoxLayout();
    oldPasswordLayout->setMargin(0);
    oldPasswordLayout->setSpacing(10);
    oldPasswordLabel->setFixedWidth(120);
    oldPasswordLayout->addWidget(oldPasswordLabel);
    oldPasswordLayout->addWidget(oldPasswordEdit);
    mainLayout->addLayout(oldPasswordLayout);

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

        // 验证旧密码
        QString oldPwd = oldPasswordEdit->text();
        QString strCipher("");
        if (!InterfaceActiveVault::checkPassword(oldPwd, strCipher)) {
            oldPasswordEdit->setAlert(true);
            oldPasswordEdit->showAlertMessage(tr("Wrong old password"), kToolTipShowDuration);
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

        // TODO: 实现重置密码的业务逻辑

        emit sigCloseDialog();
    } else {
        emit sigCloseDialog();
    }
}

void ResetPasswordByOldPasswordView::onPasswordChanged()
{
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
    newPasswordEdit->setAlert(false);
    newPasswordEdit->hideAlertMessage();
}

void ResetPasswordByOldPasswordView::onRepeatPasswordChanged(const QString &pwd)
{
    Q_UNUSED(pwd);
    onPasswordChanged();
    repeatPasswordEdit->setAlert(false);
    repeatPasswordEdit->hideAlertMessage();
}

bool ResetPasswordByOldPasswordView::checkPassword(const QString &password)
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
    newPasswordEdit->clear();
    newPasswordEdit->setAlert(false);
    newPasswordEdit->hideAlertMessage();
    repeatPasswordEdit->clear();
    repeatPasswordEdit->setAlert(false);
    repeatPasswordEdit->hideAlertMessage();
    emit sigBtnEnabled(1, false);
    QFrame::showEvent(event);
}

bool ResetPasswordByOldPasswordView::eventFilter(QObject *obj, QEvent *evt)
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

