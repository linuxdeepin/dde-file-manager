// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountaskpassworddialog.h"
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDebug>

using namespace GlobalServerDefines;

using namespace dfmbase;
DWIDGET_USE_NAMESPACE

MountAskPasswordDialog::MountAskPasswordDialog(QWidget *parent)
    : DDialog(parent)
{
    setModal(true);
    initUI();
    initConnect();
}

MountAskPasswordDialog::~MountAskPasswordDialog()
{
}

void MountAskPasswordDialog::initUI()
{
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    QStringList buttonTexts;
    buttonTexts << tr("Cancel", "button") << tr("Connect", "button");

    QFrame *content = new QFrame;

    QLabel *connectTypeLabel = new QLabel(tr("Log in as"));
    connectTypeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    connectTypeLabel->setFixedWidth(95);

    DButtonBox *anonmymousButtonBox = new DButtonBox(this);
    anonmymousButtonBox->setEnabled(true);
    anonymousButton = new DButtonBoxButton(tr("Anonymous"));
    anonymousButton->setObjectName("AnonymousButton");
    anonymousButton->setCheckable(true);
    anonymousButton->setMinimumWidth(100);
    anonymousButton->setFocusPolicy(Qt::NoFocus);
    registerButton = new DButtonBoxButton(tr("Registered user"));
    registerButton->setObjectName("RegisterButton");
    registerButton->setCheckable(true);
    registerButton->setMinimumWidth(100);
    registerButton->setFocusPolicy(Qt::NoFocus);
    registerButton->click();

    QList<DButtonBoxButton *> buttonList;
    buttonList << anonymousButton << registerButton;

    anonmymousButtonBox->setButtonList(buttonList, true);
    anonmymousButtonBox->setFocusPolicy(Qt::NoFocus);

    passwordFrame = new QFrame;

    QLabel *usernameLable = new QLabel(tr("Username"));
    usernameLable->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    usernameLable->setFixedWidth(95);

    usernameLineEdit = new QLineEdit;
    usernameLineEdit->setMinimumWidth(240);
    usernameLineEdit->setText(qgetenv("USER"));

    domainLabel = new QLabel(tr("Domain"));
    domainLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    domainLabel->setFixedWidth(95);

    domainLineEdit = new QLineEdit;
    domainLineEdit->setMinimumWidth(240);
    domainLineEdit->setText("WORKGROUP");

    QLabel *passwordLable = new QLabel(tr("Password"));
    passwordLable->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    passwordLable->setFixedWidth(90);

    passwordLineEdit = new DPasswordEdit;
    passwordLineEdit->setMinimumWidth(240);
    passwordLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    passwordButtonGroup = new QButtonGroup(this);
    passwordButtonGroup->setExclusive(true);

    passwordCheckBox = new QCheckBox();
    QWidget *empty = new QWidget();
    passwordCheckBox->setText(tr("Remember password"));

    QHBoxLayout *anonymousLayout = new QHBoxLayout;
    anonymousLayout->addWidget(anonmymousButtonBox);
    anonymousLayout->setSpacing(0);
    anonymousLayout->setContentsMargins(0, 0, 0, 0);

    QFormLayout *connectTypeLayout = new QFormLayout;
    connectTypeLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    connectTypeLayout->addRow(connectTypeLabel, anonymousLayout);

    QFormLayout *inputLayout = new QFormLayout;
    inputLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    inputLayout->setFormAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    inputLayout->addRow(usernameLable, usernameLineEdit);
    inputLayout->addRow(domainLabel, domainLineEdit);
    inputLayout->addRow(passwordLable, passwordLineEdit);
    inputLayout->addRow(empty, passwordCheckBox);
    inputLayout->setSpacing(10);

    QVBoxLayout *passwordFrameLayout = new QVBoxLayout;
    passwordFrameLayout->addLayout(inputLayout, Qt::AlignCenter);
    //    passwordFrameLayout->addWidget(m_passwordCheckBox);
    passwordFrameLayout->setSpacing(0);
    passwordFrameLayout->setContentsMargins(0, 0, 0, 0);
    passwordFrame->setLayout(passwordFrameLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addSpacing(16);
    mainLayout->addLayout(connectTypeLayout);
    mainLayout->addWidget(passwordFrame);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    content->setLayout(mainLayout);

    addContent(content);
    addButtons(buttonTexts);
    setSpacing(10);
    setDefaultButton(1);

    usernameLineEdit->setFocus();
}

void MountAskPasswordDialog::initConnect()
{
    connect(anonymousButton, &DButtonBoxButton::clicked, this, [=]() {
        passwordFrame->setVisible(false);
    });
    connect(registerButton, &DButtonBoxButton::clicked, this, [=]() {
        passwordFrame->setVisible(true);
    });
    connect(this, SIGNAL(buttonClicked(int, QString)), this, SLOT(handleButtonClicked(int, QString)));
}

QJsonObject MountAskPasswordDialog::getLoginData()
{
    return loginObj;
}

void MountAskPasswordDialog::handleConnect()
{
    using namespace NetworkMountParamKey;

    loginObj.insert(kMessage, title());

    if (anonymousButton->isChecked()) {
        loginObj.insert(kAnonymous, true);
    } else {
        loginObj.insert(kAnonymous, false);
    }

    loginObj.insert(kUser, usernameLineEdit->text());
    loginObj.insert(kDomain, domainLineEdit->text());
    loginObj.insert(kPasswd, passwordLineEdit->text());

    if (passwordCheckBox->isChecked()) {
        loginObj.insert(kPasswdSaveMode, kSavePermanently);
    } else {
        loginObj.insert(kPasswdSaveMode, kSaveBeforeLogout);   // kSaveBeforeLogout is used as default, for auth one time
    }
    accept();
}

void MountAskPasswordDialog::handleButtonClicked(int index, QString text)
{
    Q_UNUSED(text)
    if (index == 1) {
        handleConnect();
    }
}

bool MountAskPasswordDialog::getDomainLineVisible() const
{
    return domainLineVisible;
}

void MountAskPasswordDialog::setDomainLineVisible(bool domainLineVisible)
{
    if (domainLineVisible) {
        domainLabel->show();
        domainLineEdit->show();
    } else {
        domainLabel->hide();
        domainLineEdit->hide();
    }
    this->domainLineVisible = domainLineVisible;
}

void MountAskPasswordDialog::setDomain(const QString &domain)
{
    domainLineEdit->setText(domain);
}

void MountAskPasswordDialog::setUser(const QString &user)
{
    usernameLineEdit->setText(user);
}
