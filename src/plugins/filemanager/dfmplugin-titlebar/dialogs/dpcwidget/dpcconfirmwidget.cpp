// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dpcconfirmwidget.h"

#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/utils/fileutils.h>

#include <DPasswordEdit>
#include <DFloatingWidget>
#include <DToolTip>
#include <DSuggestButton>
#include <DPushButton>
#include <DVerticalLine>
#include <DFontSizeManager>
#include <DWindowManagerHelper>
#include <DSysInfo>

#include <QTimer>
#include <QHBoxLayout>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QLibrary>
#include <QDBusUnixFileDescriptor>
#include <QDataStream>

#include <unistd.h>

DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_titlebar;

namespace DaemonServiceIFace {
static constexpr char kInterfaceService[] { "org.deepin.Filemanager.AccessControlManager" };
static constexpr char kInterfacePath[] { "/org/deepin/Filemanager/AccessControlManager" };
static constexpr char kInterfaceInterface[] { "org.deepin.Filemanager.AccessControlManager" };

static constexpr char kFuncChangePwd[] { "ChangeDiskPassword" };
static constexpr char kSigPwdChecked[] { "DiskPasswordChecked" };
}   // namespace DBusInterfaceInfo

namespace DeepinPwdCheck {
// the libdeepin-pw-check-dev is required for libdeepin_pw_check.so -_-
static constexpr char kLibraryName[] { "libdeepin_pw_check.so.1" };
static constexpr char kInterfacePwdCheck[] { "deepin_pw_check" };
static constexpr char kInterfaceGetPwdLevel[] { "get_new_passwd_strength_level" };
static constexpr char kInterfaceErrToString[] { "err_to_string" };
}

static const int kPasswordMaxLength = 510;

DPCConfirmWidget::DPCConfirmWidget(QWidget *parent)
    : DWidget(parent),
      parentWidget(parent)
{
    accessControlInter.reset(new QDBusInterface(DaemonServiceIFace::kInterfaceService,
                                                DaemonServiceIFace::kInterfacePath,
                                                DaemonServiceIFace::kInterfaceInterface,
                                                QDBusConnection::systemBus(),
                                                this));

    initUI();
    initConnect();
    initLibrary();
}

void DPCConfirmWidget::initUI()
{
    titleLabel = new DLabel(tr("Change disk password"), this);
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(titleLabel, DFontSizeManager::T5, QFont::Medium);

    QRegularExpression regx("^[^\u4e00-\u9fa5]+");
    // 创建验证器
    QValidator *validator = new QRegularExpressionValidator(regx, this);

    oldPwdEdit = new DPasswordEdit(this);
    oldPwdEdit->lineEdit()->setValidator(validator);   // 设置验证器

    newPwdEdit = new DPasswordEdit(this);
    newPwdEdit->lineEdit()->setValidator(validator);

    repeatPwdEdit = new DPasswordEdit(this);
    repeatPwdEdit->lineEdit()->setValidator(validator);

    DLabel *oldPwdLabel = new DLabel(tr("Current password:"), this);
    DLabel *newPwdLabel = new DLabel(tr("New password:"), this);
    DLabel *repeatPwdLabel = new DLabel(tr("Repeat password:"), this);

    QGridLayout *contentLayout = new QGridLayout();
    contentLayout->addWidget(oldPwdLabel, 0, 0, Qt::AlignRight);
    contentLayout->addWidget(oldPwdEdit, 0, 1);
    contentLayout->addWidget(newPwdLabel, 1, 0, Qt::AlignRight);
    contentLayout->addWidget(newPwdEdit, 1, 1);
    contentLayout->addWidget(repeatPwdLabel, 2, 0, Qt::AlignRight);
    contentLayout->addWidget(repeatPwdEdit, 2, 1);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setHorizontalSpacing(10);
    contentLayout->setVerticalSpacing(10);

    saveBtn = new DSuggestButton(tr("Save", "button"), this);
    saveBtn->setAttribute(Qt::WA_NoMousePropagation);
    cancelBtn = new DPushButton(tr("Cancel", "button"), this);
    cancelBtn->setAttribute(Qt::WA_NoMousePropagation);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    DVerticalLine *line = new DVerticalLine;
    line->setFixedHeight(30);
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(line);
    buttonLayout->addWidget(saveBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(contentLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void DPCConfirmWidget::initConnect()
{
    Q_ASSERT(accessControlInter);

    connect(cancelBtn, &DPushButton::clicked, this, &DPCConfirmWidget::sigCloseDialog);
    connect(saveBtn, &DSuggestButton::clicked, this, &DPCConfirmWidget::onSaveBtnClicked);

    connect(oldPwdEdit, &DPasswordEdit::textChanged, this, &DPCConfirmWidget::onPasswdChanged);
    connect(newPwdEdit, &DPasswordEdit::textChanged, this, &DPCConfirmWidget::onPasswdChanged);
    connect(repeatPwdEdit, &DPasswordEdit::textChanged, this, &DPCConfirmWidget::onPasswdChanged);
    connect(newPwdEdit, &DPasswordEdit::editingFinished, this, &DPCConfirmWidget::onEditingFinished);

    accessControlInter->connection().connect(accessControlInter->service(),
                                             accessControlInter->path(),
                                             accessControlInter->interface(),
                                             DaemonServiceIFace::kSigPwdChecked,
                                             this,
                                             SLOT(onPasswordChecked(int)));
}

void DPCConfirmWidget::initLibrary()
{
    fmDebug() << "Loading deepin password check library:" << DeepinPwdCheck::kLibraryName;

    QLibrary lib(DeepinPwdCheck::kLibraryName);
    if (lib.load()) {
        deepinPwCheck = reinterpret_cast<DeepinPwCheckFunc>(lib.resolve(DeepinPwdCheck::kInterfacePwdCheck));
        getPasswdLevel = reinterpret_cast<GetPasswdLevelFunc>(lib.resolve(DeepinPwdCheck::kInterfaceGetPwdLevel));
        errToString = reinterpret_cast<ErrToStringFunc>(lib.resolve(DeepinPwdCheck::kInterfaceErrToString));
    } else {
        fmWarning() << "Failed to load deepin password check library:" << lib.errorString();
    }
}

void DPCConfirmWidget::showToolTips(const QString &msg, DPasswordEdit *pwdEdit)
{
    Q_ASSERT(pwdEdit);

    pwdEdit->setFocus();
    pwdEdit->showAlertMessage(msg);
}

bool DPCConfirmWidget::checkRepeatPassword()
{
    const QString &repeatPwd = repeatPwdEdit->text();
    const QString &newPwd = newPwdEdit->text();

    if (repeatPwd != newPwd) {
        fmDebug() << "Password repeat validation failed: passwords do not match";
        repeatPwdEdit->setAlert(true);
        showToolTips(tr("Passwords do not match"), repeatPwdEdit);
        return false;
    } else if (repeatPwdEdit->isAlert()) {
        repeatPwdEdit->setAlert(false);
    }

    return true;
}

bool DPCConfirmWidget::checkNewPassword()
{
    const QString &oldPwd = oldPwdEdit->text();
    const QString &newPwd = newPwdEdit->text();

    if (oldPwd == newPwd) {
        fmDebug() << "New password validation failed: new password is same as current password";
        newPwdEdit->setAlert(true);
        showToolTips(tr("New password should differ from the current one"), newPwdEdit);
        return false;
    }

    QString msg;
    if (!checkPasswdComplexity(newPwd, &msg)) {
        fmDebug() << "New password validation failed: complexity check failed";
        newPwdEdit->setAlert(true);
        showToolTips(msg, newPwdEdit);
        return false;
    }

    return true;
}

bool DPCConfirmWidget::checkPasswdComplexity(const QString &pwd, QString *msg)
{
    Q_ASSERT(msg);

    // the password complexity check is for 1060 and v23 and later
    DSysInfo::UosEdition edition = DSysInfo::uosEditionType();
    fmDebug() << "Checking password complexity for UOS edition:" << edition;

    switch (edition) {
    case DSysInfo::UosProfessional: {
        const auto &minorVer = DSysInfo::minorVersion();
        if (minorVer < "1060") {
            fmDebug() << "Skipping password complexity check for UOS Professional version:" << minorVer;
            return true;
        }
    } break;
    case DSysInfo::UosCommunity: {
        const auto &majorVer = DSysInfo::majorVersion();
        if (majorVer < "23") {
            fmDebug() << "Skipping password complexity check for UOS Community version:" << majorVer;
            return true;
        }
    } break;
    default:
        return true;
    }

    // not loaded libdeepin_pw_check.so
    if (!getPasswdLevel || !deepinPwCheck || !errToString) {
        fmWarning() << "Password complexity check library not available, skipping check";
        return true;
    }

    const auto &userName = SysInfoUtils::getUser();
    const auto &newPwdArray = pwd.toLocal8Bit();
    auto level = getPasswdLevel(newPwdArray.data());
    fmDebug() << "Password complexity level:" << level;

    if (level < 3 || userName == pwd) {
        fmDebug() << "Password complexity check failed: level too low or same as username";
        msg->append(tr("Minimum of 8 characters. At least 3 types: 0-9, a-z, A-Z and symbols. Different from the username."));
        return false;
    }

    int type = deepinPwCheck(userName.toLocal8Bit().data(), newPwdArray.data(), 3, nullptr);
    if (type != 0) {
        fmDebug() << "Password complexity check failed with error type:" << type;
        msg->append(errToString(type));
        return false;
    }

    fmDebug() << "Password complexity check passed";
    return true;
}

void DPCConfirmWidget::setEnabled(bool enabled)
{
    cancelBtn->setEnabled(enabled);
    saveBtn->setEnabled(enabled);
    if (parentWidget)
        DWindowManagerHelper::instance()->setMotifFunctions(parentWidget->windowHandle(), DWindowManagerHelper::FUNC_CLOSE, enabled);
}

void DPCConfirmWidget::onPasswdChanged()
{
    DPasswordEdit *pwdEdit = qobject_cast<DPasswordEdit *>(sender());
    if (pwdEdit && pwdEdit->isAlert())
        pwdEdit->setAlert(false);
}

void DPCConfirmWidget::onEditingFinished()
{
    DPasswordEdit *pwdEdit = qobject_cast<DPasswordEdit *>(sender());
    if (pwdEdit && pwdEdit->text().length() > kPasswordMaxLength) {
        fmDebug() << "Password length validation failed: length exceeds maximum" << kPasswordMaxLength;
        pwdEdit->setAlert(true);
        showToolTips(tr("Password must be no more than %1 characters").arg(kPasswordMaxLength), pwdEdit);
    }
}

void DPCConfirmWidget::onSaveBtnClicked()
{
    if (oldPwdEdit->text().isEmpty()) {
        fmDebug() << "Validation failed: current password is empty";
        oldPwdEdit->setAlert(true);
        showToolTips(tr("Password cannot be empty"), oldPwdEdit);
        return;
    } else if (newPwdEdit->text().isEmpty()) {
        fmDebug() << "Validation failed: new password is empty";
        newPwdEdit->setAlert(true);
        showToolTips(tr("Password cannot be empty"), newPwdEdit);
        return;
    } else if (repeatPwdEdit->text().isEmpty()) {
        fmDebug() << "Validation failed: repeat password is empty";
        repeatPwdEdit->setAlert(true);
        showToolTips(tr("Password cannot be empty"), repeatPwdEdit);
        return;
    }

    if (!checkNewPassword() || !checkRepeatPassword()) {
        fmDebug() << "Password validation failed, aborting password change";
        return;
    }

    if (accessControlInter->isValid()) {
        fmInfo() << "Sending password change request to daemon service";
        setEnabled(false);

        QString oldPass(oldPwdEdit->text().trimmed());
        QString newPass(newPwdEdit->text().trimmed());

        // Create anonymous pipe for secure credential transmission
        int pipefd[2];
        if (pipe(pipefd) == -1) {
            fmCritical() << "Failed to create anonymous pipe for credentials";
            setEnabled(true);
            return;
        }

        // Prepare credentials data using QDataStream for reliable serialization
        QByteArray credentials;
        QDataStream stream(&credentials, QIODevice::WriteOnly);
        QString oldPassEnc = FileUtils::encryptString(oldPass);
        QString newPassEnc = FileUtils::encryptString(newPass);
        stream << oldPassEnc << newPassEnc;

        // Write credentials to pipe and close write end immediately
        ssize_t written = write(pipefd[1], credentials.constData(), credentials.size());
        ::close(pipefd[1]);   // Close write end immediately after writing

        if (written != credentials.size()) {
            fmCritical() << "Failed to write credentials to pipe, written:" << written << "expected:" << credentials.size();
            ::close(pipefd[0]);
            setEnabled(true);
            return;
        }

        // Create file descriptor for D-Bus transmission
        QDBusUnixFileDescriptor fd(pipefd[0]);
        if (!fd.isValid()) {
            fmCritical() << "Failed to create valid file descriptor from pipe";
            ::close(pipefd[0]);
            setEnabled(true);
            return;
        }

        // Call D-Bus interface with pipe file descriptor
        accessControlInter->asyncCall(DaemonServiceIFace::kFuncChangePwd, QVariant::fromValue(fd));

        // Close read end (D-Bus service will have its own copy)
        ::close(pipefd[0]);
    } else {
        fmCritical() << "Access control interface is invalid, cannot change password";
    }
}

void DPCConfirmWidget::onPasswordChecked(int result)
{
    switch (result) {
    case kNoError:
        fmInfo() << "Password change completed successfully";
        emit sigConfirmed();
        break;
    case kAuthenticationFailed:
        fmWarning() << "Password change failed: authentication failed";
        setEnabled(true);
        break;
    case kPasswordWrong:
        fmWarning() << "Password change failed: wrong current password";
        setEnabled(true);
        oldPwdEdit->setAlert(true);
        showToolTips(tr("Wrong password"), oldPwdEdit);
        break;
    default:
        break;
    }
}
