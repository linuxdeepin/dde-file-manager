#include "mountaskpassworddialog.h"
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
    setFixedSize(380, 280);

    QStringList buttonTexts;
    buttonTexts << tr("Cancel") << tr("Connect");

    QFrame* content = new QFrame;

    m_messageLabel = new QLabel(this);

    QLabel* connectTypeLabel = new QLabel(tr("Log in as"));
    connectTypeLabel->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    connectTypeLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    connectTypeLabel->setFixedWidth(80);

    m_anonymousButtonGroup = new QButtonGroup(this);
    m_anonymousButtonGroup->setExclusive(true);
    QPushButton* anonymousButton = new QPushButton(tr("Anonymous"));
    anonymousButton->setObjectName("AnonymousButton");
    anonymousButton->setCheckable(true);
    anonymousButton->setFixedHeight(28);
    anonymousButton->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    QPushButton* registerButton = new QPushButton(tr("Registered user"));
    registerButton->setObjectName("RegisterButton");
    registerButton->setCheckable(true);
    registerButton->setFixedHeight(28);
    registerButton->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    m_anonymousButtonGroup->addButton(anonymousButton, 0);
    m_anonymousButtonGroup->addButton(registerButton, 1);

    m_passwordFrame = new QFrame;

    QLabel* usernameLable = new QLabel(tr("Username"));
    usernameLable->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    usernameLable->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    usernameLable->setFixedWidth(80);

    m_usernameLineEdit = new DLineEdit;
    m_usernameLineEdit->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    m_usernameLineEdit->setFixedHeight(24);

    m_domainLabel = new QLabel(tr("Domain"));
    m_domainLabel->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    m_domainLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    m_domainLabel->setFixedWidth(80);

    m_domainLineEdit = new DLineEdit;
    m_domainLineEdit->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    m_domainLineEdit->setFixedHeight(24);

    QLabel* passwordLable = new QLabel(tr("Password"));
    passwordLable->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    passwordLable->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    passwordLable->setFixedWidth(80);

    m_passwordLineEdit = new DPasswordEdit;
    QFont font;
    font.setLetterSpacing(QFont::AbsoluteSpacing, 2);
    font.setPixelSize(6);
    m_passwordLineEdit->setFont(font);
    m_passwordLineEdit->setStyleSheet(getQssFromFile(":/qss/qss/passwordAskDialog.qss"));
    m_passwordLineEdit->setFixedHeight(24);


    m_passwordButtonGroup = new QButtonGroup(this);
    m_passwordButtonGroup->setExclusive(true);


    m_passwordCheckBox = new QCheckBox();
    m_passwordCheckBox->setFixedSize(22, 22);
    QLabel* rememberLabel = new QLabel(tr("Remember password"));


    QHBoxLayout* anonymousLayout = new QHBoxLayout;
    anonymousLayout->addWidget(anonymousButton);
    anonymousLayout->addWidget(registerButton);
    anonymousLayout->setSpacing(0);
    anonymousLayout->setContentsMargins(0, 0, 0, 0);


    QFormLayout* connectTypeLayout = new  QFormLayout;
    connectTypeLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignRight);
    connectTypeLayout->addRow(connectTypeLabel, anonymousLayout);


    QFormLayout* inputLayout = new  QFormLayout;
    inputLayout->setLabelAlignment(Qt::AlignVCenter | Qt::AlignRight);
    inputLayout->setFormAlignment(Qt::AlignVCenter | Qt::AlignRight);
    inputLayout->addRow(usernameLable, m_usernameLineEdit);
    inputLayout->addRow(m_domainLabel, m_domainLineEdit);
    inputLayout->addRow(passwordLable, m_passwordLineEdit);
    inputLayout->addRow(m_passwordCheckBox, rememberLabel);
    inputLayout->setSpacing(10);

    QVBoxLayout* passwordFrameLayout = new QVBoxLayout;
    passwordFrameLayout->addLayout(inputLayout, Qt::AlignCenter);
    passwordFrameLayout->addWidget(m_passwordCheckBox);
    passwordFrameLayout->setSpacing(0);
    passwordFrameLayout->setContentsMargins(0, 0, 0, 0);
    m_passwordFrame->setLayout(passwordFrameLayout);


    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_messageLabel, Qt::AlignCenter);
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
}

void MountAskPasswordDialog::initConnect()
{
    connect(m_anonymousButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(togglePasswordFrame(int)));
    connect(this, SIGNAL(buttonClicked(int,QString)), this, SLOT(handleButtonClicked(int,QString)));
}

QJsonObject MountAskPasswordDialog::getLoginData()
{
    return m_loginObj;
}

void MountAskPasswordDialog::setLoginData(const QJsonObject &obj)
{
    m_loginObj = obj;

    QFontMetrics fm(m_messageLabel->font());
    const QString& str = fm.elidedText(m_loginObj.value("message").toString()
                                       ,Qt::ElideMiddle,
                                       this->size().width()-80);

    m_messageLabel->setText(str);

    if (m_loginObj.value("anonymous").toBool()){
        m_anonymousButtonGroup->button(0)->click();
    }else{
        m_anonymousButtonGroup->button(1)->click();
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
    m_loginObj.insert("message", m_messageLabel->text());

    if (m_anonymousButtonGroup->button(0)->isChecked()){
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

void MountAskPasswordDialog::togglePasswordFrame(int id)
{
    if (id == 0){
        setFixedSize(QSize(380, 160));
        m_passwordFrame->hide();
    }else{
        setFixedSize(QSize(380, 280));
        m_passwordFrame->show();
    }
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


