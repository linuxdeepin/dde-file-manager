// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pwdconfirmwidget.h"
#include "dbusinterface/disk_interface.h"

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

#define PASSWORD_MAX_LENGTH 512

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

PwdConfirmWidget::PwdConfirmWidget(QWidget *parent)
    : DWidget(parent),
      m_parentWidget(parent)
{
    m_diskInterface = new DiskInterface("com.deepin.filemanager.daemon",
                                        "/com/deepin/filemanager/daemon/DiskManager",
                                        QDBusConnection::systemBus(),
                                        this);

    initUI();
    initConnect();
}

void PwdConfirmWidget::initUI()
{
    m_titleLabel = new DLabel(tr("Change disk password"), this);
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(m_titleLabel, DFontSizeManager::T5, QFont::Medium);

    QRegExp regx("[A-Za-z0-9,.;?@/=()<>_- +*{}:&^%$#!`~\'\"|]+");
    // 创建验证器
    QValidator *validator = new QRegExpValidator(regx, this);

    m_oldPwdEdit = new DPasswordEdit(this);
    m_oldPwdEdit->lineEdit()->setValidator(validator);   // 设置验证器

    m_newPwdEdit = new DPasswordEdit(this);
    m_newPwdEdit->lineEdit()->setValidator(validator);

    m_repeatPwdEdit = new DPasswordEdit(this);
    m_repeatPwdEdit->lineEdit()->setValidator(validator);

    DLabel *oldPwdLabel = new DLabel(tr("Current password:"), this);
    DLabel *newPwdLabel = new DLabel(tr("New password:"), this);
    DLabel *repeatPwdLabel = new DLabel(tr("Repeat password:"), this);

    QGridLayout *contentLayout = new QGridLayout();
    contentLayout->addWidget(oldPwdLabel, 0, 0, Qt::AlignRight);
    contentLayout->addWidget(m_oldPwdEdit, 0, 1);
    contentLayout->addWidget(newPwdLabel, 1, 0, Qt::AlignRight);
    contentLayout->addWidget(m_newPwdEdit, 1, 1);
    contentLayout->addWidget(repeatPwdLabel, 2, 0, Qt::AlignRight);
    contentLayout->addWidget(m_repeatPwdEdit, 2, 1);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setHorizontalSpacing(10);
    contentLayout->setVerticalSpacing(10);

    m_saveBtn = new DSuggestButton(tr("Save"), this);
    m_saveBtn->setAttribute(Qt::WA_NoMousePropagation);
    m_cancelBtn = new DPushButton(tr("Cancel"), this);
    m_cancelBtn->setAttribute(Qt::WA_NoMousePropagation);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    DVerticalLine *line = new DVerticalLine;
    line->setFixedHeight(30);
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addWidget(line);
    buttonLayout->addWidget(m_saveBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(contentLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void PwdConfirmWidget::initConnect()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, &PwdConfirmWidget::sigClosed);
    connect(m_saveBtn, &DSuggestButton::clicked, this, &PwdConfirmWidget::onSaveBtnClicked);

    connect(m_oldPwdEdit, &DPasswordEdit::textChanged, this, &PwdConfirmWidget::checkPassword);
    connect(m_newPwdEdit, &DPasswordEdit::textChanged, this, &PwdConfirmWidget::checkPassword);
    connect(m_repeatPwdEdit, &DPasswordEdit::textChanged, this, &PwdConfirmWidget::checkPassword);

    connect(m_diskInterface, &DiskInterface::passwordChecked, this, &PwdConfirmWidget::onPasswordChecked);
}

void PwdConfirmWidget::showToolTip(const QString &msg, QWidget *w)
{
    if (!m_toolTipFrame) {
        m_toolTip = new DToolTip(msg);
        m_toolTip->setObjectName("AlertToolTip");
        m_toolTip->setWordWrap(true);
        m_toolTip->setForegroundRole(DPalette::TextWarning);

        m_toolTipFrame = new DFloatingWidget(this);
        m_toolTipFrame->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        m_toolTipFrame->setWidget(m_toolTip);
    }

    m_toolTip->setText(msg);
    if (m_toolTipFrame->parent()) {
        QFont font = m_toolTip->font();
        QFontMetrics fontMetrics(font);
        int fontWidth = fontMetrics.width(msg) + 30;
        int fontHeight = fontMetrics.lineSpacing() + 12;
        int per = fontWidth / w->width() + 1;

        QPoint p = w->mapTo(this, QPoint(0, 0));
        m_toolTipFrame->setGeometry(p.x(), p.y() + w->height(),
                                    fontWidth < w->width() ? fontWidth : w->width(),
                                    per * fontHeight);
        m_toolTipFrame->show();
        m_toolTipFrame->raise();
    }

    QTimer::singleShot(3000, this, [=]() {
        m_toolTipFrame->close();
    });
}

bool PwdConfirmWidget::checkRepeatPassword()
{
    const QString &repeatPwd = m_repeatPwdEdit->text();
    const QString &newPwd = m_newPwdEdit->text();

    if (repeatPwd != newPwd) {
        m_repeatPwdEdit->setAlert(true);
        showToolTip(tr("Passwords do not match"), m_repeatPwdEdit);
        return false;
    } else if (m_repeatPwdEdit->isAlert()) {
        m_repeatPwdEdit->setAlert(false);
    }

    return true;
}

bool PwdConfirmWidget::checkNewPassword()
{
    const QString &oldPwd = m_oldPwdEdit->text();
    const QString &newPwd = m_newPwdEdit->text();

    if (oldPwd == newPwd) {
        m_newPwdEdit->setAlert(true);
        showToolTip(tr("New password should differ from the current one"), m_newPwdEdit);
        return false;
    }

    return true;
}

void PwdConfirmWidget::checkPassword(const QString &pwd)
{
    DPasswordEdit *pwdEdit = qobject_cast<DPasswordEdit *>(sender());
    if (pwd.length() > PASSWORD_MAX_LENGTH) {
        pwdEdit->setText(pwd.mid(0, PASSWORD_MAX_LENGTH));
        pwdEdit->setAlert(true);
        showToolTip(tr("Password must be no more than 512 characters"), pwdEdit);
    } else if (pwdEdit->isAlert()) {
        pwdEdit->setAlert(false);
    }
}

void PwdConfirmWidget::onSaveBtnClicked()
{
    if (m_oldPwdEdit->text().isEmpty()) {
        m_oldPwdEdit->setAlert(true);
        showToolTip(tr("Password cannot be empty"), m_oldPwdEdit);
        return;
    } else if (m_newPwdEdit->text().isEmpty()) {
        m_newPwdEdit->setAlert(true);
        showToolTip(tr("Password cannot be empty"), m_newPwdEdit);
        return;
    } else if (m_repeatPwdEdit->text().isEmpty()) {
        m_repeatPwdEdit->setAlert(true);
        showToolTip(tr("Password cannot be empty"), m_repeatPwdEdit);
        return;
    }

    if (!checkNewPassword() || !checkRepeatPassword())
        return;

    if (m_diskInterface->isValid() && m_diskInterface->checkAuthentication()) {
        m_cancelBtn->setEnabled(false);
        m_saveBtn->setEnabled(false);
        if (m_parentWidget)
            DWindowManagerHelper::instance()->setMotifFunctions(m_parentWidget->windowHandle(), DWindowManagerHelper::FUNC_CLOSE, false);

        QByteArray byteArray = m_oldPwdEdit->text().toUtf8();
        QByteArray encodedByteArray = byteArray.toBase64();
        const QString &encodedOldPwd = QString::fromUtf8(encodedByteArray);

        byteArray = m_newPwdEditv->text().toUtf8();
        encodedByteArray = byteArray.toBase64();
        const QString &encodedNewPwd = QString::fromUtf8(encodedByteArray);

        m_diskInterface->changeDiskPassword(encodedOldPwd, encodedNewPwd);
    }
}

void PwdConfirmWidget::onPasswordChecked(bool result)
{
    if (result) {
        emit sigConfirmed();
    } else {
        m_cancelBtn->setEnabled(true);
        m_saveBtn->setEnabled(true);
        if (m_parentWidget)
            DWindowManagerHelper::instance()->setMotifFunctions(m_parentWidget->windowHandle(), DWindowManagerHelper::FUNC_CLOSE, true);

        m_oldPwdEdit->setAlert(true);
        showToolTip(tr("Wrong password"), m_oldPwdEdit);
    }
}
