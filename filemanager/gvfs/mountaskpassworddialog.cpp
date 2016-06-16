#include "mountaskpassworddialog.h"
#include <QVBoxLayout>
#include <QFormLayout>

MountAskPasswordDialog::MountAskPasswordDialog(QWidget *parent) : QDialog(parent)
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
    setFixedSize(400, 300);

    m_messageLabel = new QLabel(this);

    QLabel* connectTypeLabel = new QLabel(tr("Connect type:"));

    m_anonymousButtonGroup = new QButtonGroup(this);
    m_anonymousButtonGroup->setExclusive(true);
    QRadioButton* anonymousRadioButton = new QRadioButton(tr("anonymous user"));
    QRadioButton* registerRadioButton = new QRadioButton(tr("register user"));
    m_anonymousButtonGroup->addButton(anonymousRadioButton, 0);
    m_anonymousButtonGroup->addButton(registerRadioButton, 1);


    m_passwordFrame = new QFrame;

    QLabel* usernameLable = new QLabel(tr("username"));
    m_usernameLineEdit = new QLineEdit;

    QLabel* domainLable = new QLabel(tr("domain"));
    m_domainLineEdit = new QLineEdit;

    QLabel* passwordLable = new QLabel(tr("password"));
    m_passwordLineEdit = new QLineEdit;

    m_passwordButtonGroup = new QButtonGroup(this);
    m_passwordButtonGroup->setExclusive(true);
    QRadioButton* neverRadioButton = new QRadioButton(tr("save never"));
    QRadioButton* sessionRadioButton = new QRadioButton(tr("save for session"));
    QRadioButton* permanentRadioButton = new QRadioButton(tr("save permanently"));

    m_passwordButtonGroup->addButton(neverRadioButton, 0);
    m_passwordButtonGroup->addButton(sessionRadioButton, 1);
    m_passwordButtonGroup->addButton(permanentRadioButton, 2);

    m_cancelButton = new QPushButton(tr("Cancel"));
    m_connectButton = new QPushButton(tr("Connect"));


    QVBoxLayout* anonymousLayout = new QVBoxLayout;
    anonymousLayout->addWidget(anonymousRadioButton);
    anonymousLayout->addWidget(registerRadioButton);

    QHBoxLayout* connectLayout = new QHBoxLayout;
    connectLayout->addWidget(connectTypeLabel);
    connectLayout->addLayout(anonymousLayout);



    QFormLayout* inputLayout = new  QFormLayout;
    inputLayout->addRow(usernameLable, m_usernameLineEdit);
    inputLayout->addRow(domainLable, m_domainLineEdit);
    inputLayout->addRow(passwordLable, m_passwordLineEdit);

    QVBoxLayout* passwordLayout = new QVBoxLayout;
    passwordLayout->addWidget(neverRadioButton);
    passwordLayout->addWidget(sessionRadioButton);
    passwordLayout->addWidget(permanentRadioButton);


    QVBoxLayout* passwordFrameLayout = new QVBoxLayout;
    passwordFrameLayout->addLayout(inputLayout, Qt::AlignCenter);
    passwordFrameLayout->addLayout(passwordLayout, Qt::AlignCenter);

    m_passwordFrame->setLayout(passwordFrameLayout);


    QHBoxLayout* confirmLayout = new QHBoxLayout;
    confirmLayout->addWidget(m_cancelButton);
    confirmLayout->addWidget(m_connectButton);


    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_messageLabel, Qt::AlignCenter);
    mainLayout->addLayout(connectLayout, Qt::AlignCenter);
    mainLayout->addWidget(m_passwordFrame);
    mainLayout->addLayout(confirmLayout);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    setLayout(mainLayout);
}

void MountAskPasswordDialog::initConnect()
{
    connect(m_cancelButton, &QPushButton::clicked, this, &MountAskPasswordDialog::reject);
    connect(m_connectButton, &QPushButton::clicked, this, &MountAskPasswordDialog::handleConnect);
}

QJsonObject MountAskPasswordDialog::getLoginData()
{
    return m_loginObj;
}

void MountAskPasswordDialog::setLoginData(const QJsonObject &obj)
{
    m_loginObj = obj;

    m_messageLabel->setText(m_loginObj.value("message").toString());

    if (m_loginObj.value("anonymous").toBool()){
        m_anonymousButtonGroup->button(0)->setChecked(true);
    }else{
        m_anonymousButtonGroup->button(1)->setChecked(true);
    }

    m_usernameLineEdit->setText(m_loginObj.value("username").toString());
    m_domainLineEdit->setText(m_loginObj.value("domain").toString());
    m_passwordLineEdit->setText(m_loginObj.value("password").toString());

    if (m_loginObj.value("passwordSave").toInt() == 0){
        m_passwordButtonGroup->button(0)->setChecked(true);
    }else if (m_loginObj.value("passwordSave").toInt() == 1){
        m_passwordButtonGroup->button(1)->setChecked(true);
    }else if (m_loginObj.value("passwordSave").toInt() == 2){
        m_passwordButtonGroup->button(2)->setChecked(true);
    }

}

void MountAskPasswordDialog::handleConnect()
{
    m_loginObj.insert("message", m_messageLabel->text());

    if (m_anonymousButtonGroup->button(0)->isChecked()){
        m_loginObj.insert("anonymous", true);
    }else{
        m_loginObj.insert("anonymous", false);
    }

    m_loginObj.insert("username", m_usernameLineEdit->text());
    m_loginObj.insert("domain", m_domainLineEdit->text());
    m_loginObj.insert("password", m_passwordLineEdit->text());

    m_loginObj.insert("passwordSave", m_passwordButtonGroup->checkedId());

    accept();
}


