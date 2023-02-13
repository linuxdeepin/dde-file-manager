// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dpcconfirmwidget.h"

#include <DPasswordEdit>
#include <DFloatingWidget>
#include <DToolTip>
#include <DSuggestButton>
#include <DPushButton>
#include <DVerticalLine>
#include <DFontSizeManager>
#include <DWindowManagerHelper>

#include <QTimer>
#include <QHBoxLayout>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingCall>

static const int kPasswordMaxLength = 512;

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
}

void DPCConfirmWidget::initUI()
{
    titleLabel = new DLabel(tr("Change disk password"), this);
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(titleLabel, DFontSizeManager::T5, QFont::Medium);

    QRegExp regx("[A-Za-z0-9,.;?@/=()<>_- +*{}:&^%$#!`~\'\"|]+");
    // 创建验证器
    QValidator *validator = new QRegExpValidator(regx, this);

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
    mainLayout->setMargin(0);
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

    connect(oldPwdEdit, &DPasswordEdit::textChanged, this, &DPCConfirmWidget::checkPasswordLength);
    connect(newPwdEdit, &DPasswordEdit::textChanged, this, &DPCConfirmWidget::checkPasswordLength);
    connect(repeatPwdEdit, &DPasswordEdit::textChanged, this, &DPCConfirmWidget::checkPasswordLength);

    accessControlInter->connection().connect(accessControlInter->service(),
                                             accessControlInter->path(),
                                             accessControlInter->interface(),
                                             DaemonServiceIFace::kSigPwdChecked,
                                             this,
                                             SLOT(onPasswordChecked(int)));
}

void DPCConfirmWidget::showToolTips(const QString &msg, QWidget *w)
{
    if (!toolTipFrame) {
        toolTip = new DToolTip(msg);
        toolTip->setObjectName("AlertToolTip");
        toolTip->setWordWrap(true);
        toolTip->setForegroundRole(DPalette::TextWarning);

        toolTipFrame = new DFloatingWidget(this);
        toolTipFrame->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        toolTipFrame->setWidget(toolTip);
    }

    toolTip->setText(msg);
    if (toolTipFrame->parent()) {
        QFont font = toolTip->font();
        QFontMetrics fontMetrics(font);
        int fontWidth = fontMetrics.horizontalAdvance(msg) + 30;
        int fontHeight = fontMetrics.lineSpacing() + 12;
        int per = fontWidth / w->width() + 1;

        QPoint p = w->mapTo(this, QPoint(0, 0));
        toolTipFrame->setGeometry(p.x(), p.y() + w->height(),
                                  fontWidth < w->width() ? fontWidth : w->width(),
                                  per * fontHeight);
        toolTipFrame->show();
        toolTipFrame->raise();
    }

    QTimer::singleShot(3000, this, [=]() {
        toolTipFrame->close();
    });
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

    return true;
}

void DPCConfirmWidget::setEnabled(bool enabled)
{
    cancelBtn->setEnabled(enabled);
    saveBtn->setEnabled(enabled);
    if (parentWidget)
        DWindowManagerHelper::instance()->setMotifFunctions(parentWidget->windowHandle(), DWindowManagerHelper::FUNC_CLOSE, enabled);
}

void DPCConfirmWidget::checkPasswordLength(const QString &pwd)
{
    DPasswordEdit *pwdEdit = qobject_cast<DPasswordEdit *>(sender());
    if (pwd.length() > kPasswordMaxLength) {
        pwdEdit->setText(pwd.mid(0, kPasswordMaxLength));
        pwdEdit->setAlert(true);
        showToolTips(tr("Password must be no more than 512 characters"), pwdEdit);
    } else if (pwdEdit->isAlert()) {
        pwdEdit->setAlert(false);
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
