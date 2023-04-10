// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usersharepasswordsettingdialog.h"
#include "dbusinterface/usershare_interface.h"
#include "../app/define.h"
#include "singleton.h"
#include "usershare/usersharemanager.h"
#include <QDebug>
#include <QProcess>
#include <QDBusReply>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWindow>

UserSharePasswordSettingDialog::UserSharePasswordSettingDialog(QWidget *parent) : DDialog(parent)
{
    setTitle(tr("Enter a password to protect shared folders"));
    setIcon(QIcon::fromTheme("dialog-password-publicshare"));
    initUI();
}

void UserSharePasswordSettingDialog::initUI()
{
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel","button"));
    buttonTexts.append(tr("Confirm","button"));
    addButton(buttonTexts[0], false);
    addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    setDefaultButton(1);
    m_passwordEdit = new DPasswordEdit(this);
    m_passwordEdit->setFocus();
    QWidget* widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    widget->setLayout(layout);
    layout->addWidget(m_passwordEdit);
    QPalette pe;
    pe.setColor(QPalette::WindowText, QColor("#526A7F"));
    QLabel* notes = new QLabel(tr("Set a password on the shared folder for non-anonymous access"),this);
    QFontMetrics fm = QFontMetrics( notes->font());
    qInfo()<<fm.width(notes->text());
    qInfo()<<this->width();
    if (fm.width(notes->text()) > this->width()){
        notes->setToolTip(notes->text());
        notes->setText(QFontMetrics(notes->font()).elidedText(notes->text(), Qt::ElideRight, fm.width(notes->text()) - 70));
    }
    notes->setAttribute(Qt::WA_TranslucentBackground, true);
    notes->setPalette(pe);
    layout->addWidget(notes);
    addContent(widget);
    setContentsMargins(0,0,0,0);
    getButton(1)->setEnabled(false);

    connect(m_passwordEdit,&DPasswordEdit::textChanged,this,[this]{
        getButton(1)->setEnabled(!m_passwordEdit->text().isEmpty());
    });

    if(DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
        this->setFixedSize(QSize(390, 210));
    }

    setTabOrder(m_passwordEdit, getButton(0));
    setTabOrder(getButton(0), this);
}

void UserSharePasswordSettingDialog::showEvent(QShowEvent *event)
{
    m_passwordEdit->setFocus();
    DDialog::showEvent(event);
}

void UserSharePasswordSettingDialog::onButtonClicked(const int &index)
{
    if (index == 1) {
        // set usershare password
        QString password = m_passwordEdit->text();
        if (password.isEmpty()) {
            close();
            return;
        }
        userShareManager->setSambaPassword(UserShareManager::getCurrentUserName(), password);
    }
    close();
}
