// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dpcconfirmwidget.h"

#include <dfm-base/utils/sysinfoutils.h>

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

DCORE_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
using namespace dfmplugin_titlebar;

namespace DaemonServiceIFace {
static constexpr char kInterfaceService[] { "com.deepin.filemanager.daemon" };
static constexpr char kInterfacePath[] { "/com/deepin/filemanager/daemon/AccessControlManager" };
static constexpr char kInterfaceInterface[] { "com.deepin.filemanager.daemon.AccessControlManager" };

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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRegExp regx("[^\\x4e00-\\x9fa5]+");
    // 创建验证器
    QValidator *validator = new QRegExpValidator(regx, this);
#else
    QRegularExpression regx("[^\\x4e00-\\x9fa5]+");
    // 创建验证器
    QValidator *validator = new QRegularExpressionValidator(regx, this);
#endif

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mainLayout->setMargin(0);
#else
    mainLayout->setContentsMargins(0, 0, 0, 0);
#endif
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
    QLibrary lib(DeepinPwdCheck::kLibraryName);
    if (lib.load()) {
        deepinPwCheck = reinterpret_cast<DeepinPwCheckFunc>(lib.resolve(DeepinPwdCheck::kInterfacePwdCheck));
        getPasswdLevel = reinterpret_cast<GetPasswdLevelFunc>(lib.resolve(DeepinPwdCheck::kInterfaceGetPwdLevel));
        errToString = reinterpret_cast<ErrToStringFunc>(lib.resolve(DeepinPwdCheck::kInterfaceErrToString));
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
        newPwdEdit->setAlert(true);
        showToolTips(tr("New password should differ from the current one"), newPwdEdit);
        return false;
    }

    QString msg;
    if (!checkPasswdComplexity(newPwd, &msg)) {
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
    switch (edition) {
    case DSysInfo::UosProfessional: {
        const auto &minorVer = DSysInfo::minorVersion();
        if (minorVer < "1060")
            return true;
    } break;
    case DSysInfo::UosCommunity: {
        const auto &majorVer = DSysInfo::majorVersion();
        if (majorVer < "23")
            return true;
    } break;
    default:
        return true;
    }

    // not loaded libdeepin_pw_check.so
    if (!getPasswdLevel || !deepinPwCheck || !errToString)
        return true;

    const auto &userName = SysInfoUtils::getUser();
    const auto &newPwdArray = pwd.toLocal8Bit();
    auto level = getPasswdLevel(newPwdArray.data());
    if (level < 3 || userName == pwd) {
        msg->append(tr("Minimum of 8 characters. At least 3 types: 0-9, a-z, A-Z and symbols. Different from the username."));
        return false;
    }

    int type = deepinPwCheck(userName.toLocal8Bit().data(), newPwdArray.data(), 3, nullptr);
    if (type != 0) {
        msg->append(errToString(type));
        return false;
    }

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
        pwdEdit->setAlert(true);
        showToolTips(tr("Password must be no more than %1 characters").arg(kPasswordMaxLength), pwdEdit);
    }
}

void DPCConfirmWidget::onSaveBtnClicked()
{
    if (oldPwdEdit->text().isEmpty()) {
        oldPwdEdit->setAlert(true);
        showToolTips(tr("Password cannot be empty"), oldPwdEdit);
        return;
    } else if (newPwdEdit->text().isEmpty()) {
        newPwdEdit->setAlert(true);
        showToolTips(tr("Password cannot be empty"), newPwdEdit);
        return;
    } else if (repeatPwdEdit->text().isEmpty()) {
        repeatPwdEdit->setAlert(true);
        showToolTips(tr("Password cannot be empty"), repeatPwdEdit);
        return;
    }

    if (!checkNewPassword() || !checkRepeatPassword())
        return;

    if (accessControlInter->isValid()) {
        setEnabled(false);
        accessControlInter->asyncCall(DaemonServiceIFace::kFuncChangePwd, oldPwdEdit->text(), newPwdEdit->text());
    }
}

void DPCConfirmWidget::onPasswordChecked(int result)
{
    switch (result) {
    case kNoError:
        emit sigConfirmed();
        break;
    case kAuthenticationFailed:
        setEnabled(true);
        break;
    case kPasswordWrong:
        setEnabled(true);
        oldPwdEdit->setAlert(true);
        showToolTips(tr("Wrong password"), oldPwdEdit);
        break;
    default:
        break;
    }
}
