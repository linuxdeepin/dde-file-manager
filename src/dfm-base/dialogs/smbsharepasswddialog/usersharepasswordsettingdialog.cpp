// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usersharepasswordsettingdialog.h"
#include <dfm-base/utils/windowutils.h>

//#include <dfm-framework/event/event.h>

#include <dtkcore_global.h>
#include <DLabel>
#include <DFontSizeManager>

#include <QObject>
#include <QDebug>
#include <QProcess>
#include <QDBusReply>
#include <QVBoxLayout>

DFMBASE_USE_NAMESPACE
using namespace dfmbase;
DWIDGET_USE_NAMESPACE

UserSharePasswordSettingDialog::UserSharePasswordSettingDialog(QWidget *parent)
    : DDialog(parent)
{
    setTitle(tr("Enter a password to protect shared folders"));
    setIcon(QIcon::fromTheme("dialog-password-publicshare"));
    initializeUi();
}

void UserSharePasswordSettingDialog::initializeUi()
{
    QStringList buttonTexts;
    buttonTexts.append(QObject::tr("Cancel", "button"));
    buttonTexts.append(QObject::tr("Confirm", "button"));
    addButton(buttonTexts[0], false);
    addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    setDefaultButton(1);
    passwordEdit = new Dtk::Widget::DPasswordEdit(this);
    addContent(passwordEdit);
    setContentsMargins(0, 0, 0, 0);
    getButton(1)->setEnabled(false);
    DLabel *notice = new DLabel(tr("Set a password on the shared folder for non-anonymous access"), this);
    QPalette pe;
    pe.setColor(QPalette::WindowText, QColor("#526A7F"));
    notice->setMargin(5);
    notice->setPalette(pe);
    DFontSizeManager::instance()->bind(notice, DFontSizeManager::SizeType::T8);
    insertContent(1, notice);

    connect(passwordEdit, &Dtk::Widget::DPasswordEdit::textChanged, this, [this] {
        getButton(1)->setEnabled(!passwordEdit->text().isEmpty());
    });

    if (WindowUtils::isWayLand()) {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        // this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
        this->setFixedSize(QSize(390, 210));
    }

    setTabOrder(passwordEdit, getButton(0));
    setTabOrder(getButton(0), this);
    passwordEdit->setFocus();
}

void UserSharePasswordSettingDialog::onButtonClicked(const int &index)
{
    if (index == 1) {
        // set usershare password
        QString password = passwordEdit->text();
        if (password.isEmpty()) {
            close();
            return;
        }
        Q_EMIT inputPassword(password);
    }
    close();
}
