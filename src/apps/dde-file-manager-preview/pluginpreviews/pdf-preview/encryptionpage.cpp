// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encryptionpage.h"

#include <DFontSizeManager>
#include <DApplication>
#include <DLabel>
#include <DGuiApplicationHelper>

#include <QBoxLayout>

DWIDGET_USE_NAMESPACE
using namespace plugin_filepreview;
EncryptionPage::EncryptionPage(QWidget *parent)
    : QWidget(parent)
{
    InitUI();
    InitConnection();
}

EncryptionPage::~EncryptionPage()
{
}

void EncryptionPage::InitUI()
{
    QPixmap m_encrypticon = QIcon::fromTheme("dr_compress_lock").pixmap(128, 128);
    DLabel *pixmaplabel = new DLabel(this);
    pixmaplabel->setPixmap(m_encrypticon);
    DLabel *stringinfolabel = new DLabel(this);

    DFontSizeManager::instance()->bind(stringinfolabel, DFontSizeManager::T5, QFont::DemiBold);
    stringinfolabel->setForegroundRole(DPalette::ToolTipText);
    stringinfolabel->setText(tr("Encrypted file, please enter the password"));

    passwordEdit = new DPasswordEdit(this);
    passwordEdit->setFixedSize(360, 36);
    QLineEdit *edit = passwordEdit->lineEdit();
    edit->setObjectName("passwdEdit");
    edit->setPlaceholderText(tr("Password"));

    nextbutton = new DPushButton(this);
    nextbutton->setObjectName("ensureBtn");
    nextbutton->setFixedSize(360, 36);
    nextbutton->setText(tr("OK", "button"));
    nextbutton->setDisabled(true);

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->setSpacing(0);
    mainlayout->addStretch();
    mainlayout->addWidget(pixmaplabel, 0, Qt::AlignCenter);

    mainlayout->addSpacing(4);
    mainlayout->addWidget(stringinfolabel, 0, Qt::AlignCenter);

    mainlayout->addSpacing(30);
    mainlayout->addWidget(passwordEdit, 0, Qt::AlignCenter);

    mainlayout->addSpacing(20);
    mainlayout->addWidget(nextbutton, 0, Qt::AlignCenter);

    mainlayout->addStretch();

    setAutoFillBackground(true);

    onUpdateTheme();
    passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &EncryptionPage::onUpdateTheme);
}

void EncryptionPage::InitConnection()
{
    connect(passwordEdit, &DPasswordEdit::textChanged, this, &EncryptionPage::onPasswordChanged);
    connect(nextbutton, &DPushButton::clicked, this, &EncryptionPage::nextbuttonClicked);

    connect(qApp, SIGNAL(sigSetPasswdFocus()), this, SLOT(onSetPasswdFocus()));
}

void EncryptionPage::nextbuttonClicked()
{
    emit sigExtractPassword(passwordEdit->text());
}

void EncryptionPage::wrongPassWordSlot()
{
    passwordEdit->clear();
    passwordEdit->setAlert(true);
    passwordEdit->showAlertMessage(tr("Wrong password"));
    passwordEdit->lineEdit()->setFocus(Qt::TabFocusReason);
}

void EncryptionPage::onPasswordChanged()
{
    if (passwordEdit->isAlert()) {
        passwordEdit->setAlert(false);
        passwordEdit->hideAlertMessage();
    }

    if (passwordEdit->text().isEmpty()) {
        nextbutton->setDisabled(true);
    } else {
        nextbutton->setEnabled(true);
    }
}

void EncryptionPage::onSetPasswdFocus()
{
    if (this->isVisible() && passwordEdit)
        passwordEdit->lineEdit()->setFocus(Qt::TabFocusReason);
}

void EncryptionPage::onUpdateTheme()
{
    DPalette plt = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette();
    plt.setColor(Dtk::Gui::DPalette::Window, plt.color(Dtk::Gui::DPalette::Base));
    setPalette(plt);
}
