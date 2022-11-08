// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mountaskpassworddialog.h"
#include "shutil/smbintegrationswitcher.h"
#include "../app/define.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QDebug>
#include "utils.h"

MountAskPasswordDialog::MountAskPasswordDialog(QWidget *parent) : DDialog(parent)
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
    buttonTexts << tr("Cancel","button") << tr("Connect","button");

    QFrame* content = new QFrame;

    QLabel* connectTypeLabel = new QLabel(tr("Log in as"));
    connectTypeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    connectTypeLabel->setFixedWidth(80);

    DButtonBox *anonmymousButtonBox = new DButtonBox(this);
    anonmymousButtonBox->setEnabled(true);
    m_anonymousButton = new DButtonBoxButton(tr("Anonymous"));
    m_anonymousButton->setObjectName("AnonymousButton");
    m_anonymousButton->setCheckable(true);
    m_anonymousButton->setMinimumWidth(100);
    m_anonymousButton->setFocusPolicy(Qt::NoFocus);
    m_registerButton = new DButtonBoxButton(tr("Registered user"));
    m_registerButton->setObjectName("RegisterButton");
    m_registerButton->setCheckable(true);
    m_registerButton->setMinimumWidth(100);
    m_registerButton->setFocusPolicy(Qt::NoFocus);

    QList<DButtonBoxButton*> buttonList;
    buttonList << m_anonymousButton << m_registerButton;

    anonmymousButtonBox->setButtonList(buttonList, true);
    anonmymousButtonBox->setFocusPolicy(Qt::NoFocus);

    m_passwordFrame = new QFrame;

    QLabel* usernameLable = new QLabel(tr("Username"));
    usernameLable->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    usernameLable->setFixedWidth(80);

    m_usernameLineEdit = new QLineEdit;
    m_usernameLineEdit->setMinimumWidth(240);

    m_domainLabel = new QLabel(tr("Domain"));
    m_domainLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_domainLabel->setFixedWidth(80);

    m_domainLineEdit = new QLineEdit;
    m_domainLineEdit->setMinimumWidth(240);

    QLabel* passwordLable = new QLabel(tr("Password"));
    passwordLable->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    passwordLable->setFixedWidth(80);

    m_passwordLineEdit = new DPasswordEdit;
    m_passwordLineEdit->setMinimumWidth(240);
    m_passwordLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    m_passwordButtonGroup = new QButtonGroup(this);
    m_passwordButtonGroup->setExclusive(true);

    m_passwordCheckBox = new QCheckBox();
    QWidget *empty = new QWidget();
    m_passwordCheckBox->setText(tr("Remember password"));

    QHBoxLayout* anonymousLayout = new QHBoxLayout;
    anonymousLayout->addWidget(anonmymousButtonBox);
    anonymousLayout->setSpacing(0);
    anonymousLayout->setContentsMargins(0, 0, 0, 0);

    QFormLayout* connectTypeLayout = new  QFormLayout;
    connectTypeLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    connectTypeLayout->addRow(connectTypeLabel, anonymousLayout);

    QFormLayout* inputLayout = new  QFormLayout;
    inputLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    inputLayout->setFormAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    inputLayout->addRow(usernameLable, m_usernameLineEdit);
    inputLayout->addRow(m_domainLabel, m_domainLineEdit);
    inputLayout->addRow(passwordLable, m_passwordLineEdit);
    inputLayout->addRow(empty, m_passwordCheckBox);
    inputLayout->setSpacing(10);

    QVBoxLayout* passwordFrameLayout = new QVBoxLayout;
    passwordFrameLayout->addLayout(inputLayout, Qt::AlignCenter);
//    passwordFrameLayout->addWidget(m_passwordCheckBox);
    passwordFrameLayout->setSpacing(0);
    passwordFrameLayout->setContentsMargins(0, 0, 0, 0);
    m_passwordFrame->setLayout(passwordFrameLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addSpacing(16);
    mainLayout->addLayout(connectTypeLayout);
    mainLayout->addWidget(m_passwordFrame);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 20, 0);
    content->setLayout(mainLayout);

    addContent(content);
    addButtons(buttonTexts);
    setSpacing(10);
    setDefaultButton(1);
    QTimer::singleShot(0,this,[this](){
        this->moveToCenter(); // 因networkmanager.cpp中while循环中局部事件循环的存在，导致对话框不能移到中央，这里采用QTimer触发
    });
}

void MountAskPasswordDialog::initConnect()
{
    connect(m_anonymousButton, &DButtonBoxButton::clicked, this, [=](){
        m_passwordFrame->setVisible(false);
    });
    connect(m_registerButton, &DButtonBoxButton::clicked, this, [=](){
        m_passwordFrame->setVisible(true);
    });
    connect(this, SIGNAL(buttonClicked(int,QString)), this, SLOT(handleButtonClicked(int,QString)));
}

QJsonObject MountAskPasswordDialog::getLoginData()
{
    return m_loginObj;
}

void MountAskPasswordDialog::setLoginData(const QJsonObject &obj)
{
    m_loginObj = obj;

    QFontMetrics fm(this->font());
    const QString &str = fm.elidedText(m_loginObj.value("message").toString()
                                       ,Qt::ElideMiddle,
                                       this->size().width()-80);

    setTitle(str);

    if (m_loginObj.value("anonymous").toBool()) {
        m_anonymousButton->click();
    } else {
        m_registerButton->click();
    }

    m_usernameLineEdit->setText(m_loginObj.value("username").toString());
    m_domainLineEdit->setText(m_loginObj.value("domain").toString());
    m_passwordLineEdit->setText(m_loginObj.value("password").toString());

    if (m_loginObj.value("passwordSave").toInt() == 2){
        m_passwordCheckBox->setChecked(true);
    }else{
        m_passwordCheckBox->setChecked(false);
    }

}

void MountAskPasswordDialog::handleConnect()
{
    m_loginObj.insert("message", title());

    if (m_anonymousButton->isChecked()){
        m_loginObj.insert("anonymous", true);
    }else{
        m_loginObj.insert("anonymous", false);
    }

    m_loginObj.insert("username", m_usernameLineEdit->text());
    m_loginObj.insert("domain", m_domainLineEdit->text());
    m_loginObj.insert("password", m_passwordLineEdit->text());

    if(m_passwordCheckBox->isChecked()){
        m_loginObj.insert("passwordSave", 2);
    }else{
        m_loginObj.insert("passwordSave", 0);
    }
    accept();
}

void MountAskPasswordDialog::handleButtonClicked(int index, QString text)
{
    Q_UNUSED(text)
    if (index == 1){
        handleConnect();
    }
}

bool MountAskPasswordDialog::getDomainLineVisible() const
{
    return m_domainLineVisible;
}

void MountAskPasswordDialog::setDomainLineVisible(bool domainLineVisible)
{
    if (domainLineVisible){
        m_domainLabel->show();
        m_domainLineEdit->show();
    }else{
        m_domainLabel->hide();
        m_domainLineEdit->hide();
    }
    m_domainLineVisible = domainLineVisible;
}


