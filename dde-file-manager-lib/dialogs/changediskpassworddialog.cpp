/*
* Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
*
* Author: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* Maintainer: Liu Zhangjian<liuzhangjian@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#include "changediskpassworddialog.h"
#include "dbusinterface/disk_interface.h"
#include "dialogmanager.h"
#include "app/define.h"

#include <DPasswordEdit>
#include <DToolTip>
#include <DFloatingWidget>
#include <dpicturesequenceview.h>

#include <QLabel>
#include <QGridLayout>
#include <QTimer>
#include <QHideEvent>
#include <QAbstractButton>
#include <QFontMetrics>

#define PASSWORD_LENGHT_MAX     512

enum VerifyInfo
{
    Success,
    ConfirmFailed,
    InitCryptFailed,
    DeviceIsEmpty,
    WrongPassword,
    CryptLoadFailed,
    Unknown
};

WaitWork::WaitWork(QWidget *parent)
    : QWidget (parent)
{
    setFixedSize(60, 60);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);//设置为对话框风格，并且去掉边框
    setWindowModality(Qt::WindowModal);//设置为模式对话框，同时在构造该对话框时要设置父窗口

    initUI();
    showAction();
}

WaitWork::~WaitWork()
{

}

void WaitWork::initUI()
{
    m_loadingIndicator = new DPictureSequenceView(this);
    m_loadingIndicator->setFixedSize(38, 38);
    m_loadingIndicator->setSpeed(20);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_loadingIndicator, 1, Qt::AlignCenter);
    setLayout(layout);
}

void WaitWork::showAction()
{
    QStringList seq;
    for (int i(1); i != 91; ++i)
        seq.append(QString(":/images/images/Spinner/Spinner%1.png").arg(i, 2, 10, QChar('0')));
    m_loadingIndicator->setPictureSequence(seq, true);
    m_loadingIndicator->play();
}

ChangeDiskPasswordDialog::ChangeDiskPasswordDialog(QWidget *parent)
    : DDialog(parent)
{
    setTitle(tr("Change disk password"));
    setIcon(QIcon::fromTheme("dialog-warning"));

    m_diskInterface = new DiskInterface("com.deepin.filemanager.daemon",
                                        "/com/deepin/filemanager/daemon/DiskManager",
                                        QDBusConnection::systemBus(),
                                        this);
    initUI();
    initConnect();
}

ChangeDiskPasswordDialog::~ChangeDiskPasswordDialog()
{
    if (m_toolTip)
        m_toolTip->deleteLater();
}

void ChangeDiskPasswordDialog::initUI()
{
    setFixedSize(420, 270);
    QStringList buttonTexts({tr("Cancel"), tr("Save")});
    addButton(buttonTexts[CancelButton], false);
    addButton(buttonTexts[SaveButton], true, DDialog::ButtonRecommend);
    setOnButtonClickedClose(false);

    QFrame *contentFrame = new QFrame(this);
    QRegExp regx("[A-Za-z0-9,.;?@/=()<>_- +*{}:&^%$#!`~\'\"|]+");
    // 创建验证器
    QValidator *validator = new QRegExpValidator(regx, this);

    m_oldPwdEdit = new DPasswordEdit(this);
    m_oldPwdEdit->lineEdit()->setValidator(validator);   // 设置验证器

    m_newPwdEdit = new DPasswordEdit(this);
    m_newPwdEdit->lineEdit()->setValidator(validator);

    m_repeatPwdEdit = new DPasswordEdit(this);
    m_repeatPwdEdit->lineEdit()->setValidator(validator);

    QLabel *oldPwdLabel = new QLabel(tr("Current password:"), this);
    QLabel *newPwdLabel = new QLabel(tr("New password:"), this);
    QLabel *repeatPwdLabel = new QLabel(tr("Repeat password:"), this);

    QGridLayout *contentLayout = new QGridLayout();
    contentLayout->setContentsMargins(0, 20, 0, 0);

    contentLayout->addWidget(oldPwdLabel, 0, 0, 1, 1, Qt::AlignRight);
    contentLayout->addWidget(m_oldPwdEdit, 0, 1, 1, 5);
    contentLayout->addWidget(newPwdLabel, 1, 0, 1, 1, Qt::AlignRight);
    contentLayout->addWidget(m_newPwdEdit, 1, 1, 1, 5);
    contentLayout->addWidget(repeatPwdLabel, 2, 0, 1, 1, Qt::AlignRight);
    contentLayout->addWidget(m_repeatPwdEdit, 2, 1, 1, 5);

    contentFrame->setLayout(contentLayout);
    addContent(contentFrame);
    setContentsMargins(0, 0, 0, 0);

    m_wait = new WaitWork(this);    
 }

void ChangeDiskPasswordDialog::initConnect()
{
    connect(m_oldPwdEdit, &DPasswordEdit::textChanged, this, &ChangeDiskPasswordDialog::checkPassword);

    connect(m_newPwdEdit, &DPasswordEdit::textChanged, this, &ChangeDiskPasswordDialog::checkPassword);

    connect(m_repeatPwdEdit, &DPasswordEdit::textChanged, this, &ChangeDiskPasswordDialog::checkPassword);

    connect(this, &ChangeDiskPasswordDialog::buttonClicked, this, &ChangeDiskPasswordDialog::onButtonClicked);
    connect(m_diskInterface, &DiskInterface::confirmed, this, &ChangeDiskPasswordDialog::onConfirmed);
    connect(m_diskInterface, &DiskInterface::finished, this, &ChangeDiskPasswordDialog::onFinished);
}

void ChangeDiskPasswordDialog::showToolTip(const QString &msg, QWidget *w)
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

void ChangeDiskPasswordDialog::checkPassword(const QString &pwd)
{
    DPasswordEdit *pwdEdit = qobject_cast<DPasswordEdit *>(sender());
    if (pwd.length() > PASSWORD_LENGHT_MAX) {
        pwdEdit->setText(pwd.mid(0, PASSWORD_LENGHT_MAX));
        pwdEdit->setAlert(true);
        showToolTip(tr("Password must be no more than 512 characters"), pwdEdit);
    } else if (pwdEdit->isAlert()) {
        pwdEdit->setAlert(false);
    }
}

bool ChangeDiskPasswordDialog::checkRepeatPassword()
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

bool ChangeDiskPasswordDialog::checkNewPassword()
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

void ChangeDiskPasswordDialog::onButtonClicked(const int &index)
{
    if (SaveButton == index) {
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

        if (m_diskInterface->isValid())
            m_diskInterface->changeDiskPassword(m_oldPwdEdit->text(), m_newPwdEdit->text());

        return;
    }

    close();
}

void ChangeDiskPasswordDialog::onConfirmed(bool state)
{
    if (state) {
        m_wait->show();
        m_wait->adjustSize();
    }
}

void ChangeDiskPasswordDialog::onFinished(int code)
{
    m_wait->close();

    switch (code) {
    case WrongPassword:
        m_oldPwdEdit->setAlert(true);
        showToolTip(tr("Wrong password"), m_oldPwdEdit);
        return;
    case ConfirmFailed:
        return;
    case InitCryptFailed:
    case DeviceIsEmpty:
    case CryptLoadFailed:
        dialogManager->showMessageDialog(3, tr("Initialization failed, error: %1").arg(code));
        return;
    default:
        break;
    }

    close();
}
