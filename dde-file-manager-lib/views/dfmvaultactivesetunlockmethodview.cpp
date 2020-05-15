#include "dfmvaultactivesetunlockmethodview.h"
#include "operatorcenter.h"

#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDebug>
#include <QToolTip>
#include <QRegExp>
#include <QRegExpValidator>
#include <QLabel>
#include <QGridLayout>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <DPasswordEdit>

DWIDGET_USE_NAMESPACE

DFMVaultActiveSetUnlockMethodView::DFMVaultActiveSetUnlockMethodView(QWidget *parent)
    : QWidget(parent)
    , m_pTypeCombo(nullptr)
    , m_pPassword(nullptr)
    , m_pRepeatPassword(nullptr)
    , m_pPasswordHintLabel(nullptr)
    , m_pTips(nullptr)
    , m_pOtherMethod(nullptr)
    , m_pNext(nullptr)
{
    // 设置开锁方式标签
    QLabel *pLabel = new QLabel(tr("Set Vault Password"), this);
    pLabel->setStyleSheet("font: 16pt 'CESI黑体-GB13000'");

    // 类型
    QLabel *pTypeLabel = new QLabel(tr("Method"), this);
    m_pTypeCombo = new QComboBox(this);
    QStringList lstItems;
    lstItems << tr("Manual")/* << tr("Random")*/;
    m_pTypeCombo->addItems(lstItems);
    connect(m_pTypeCombo, SIGNAL(currentIndexChanged(int)),
                      this, SLOT(slotTypeChanged(int)));

    // 设置密码
    m_pPasswordLabel = new QLabel(tr("Password"),this);
    m_pPassword = new DPasswordEdit(this);
    m_pPassword->lineEdit()->setPlaceholderText(tr("At least 8 characters, and contain A-Z, a-z, 0-9, and symbols"));
    connect(m_pPassword, &DPasswordEdit::textEdited,
            this, &DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(m_pPassword, &DPasswordEdit::editingFinished,
            this, &DFMVaultActiveSetUnlockMethodView::slotPasswordEditFinished);


    // 重复密码
    m_pRepeatPasswordLabel = new QLabel(tr("Repeat password"), this);
    m_pRepeatPassword = new DPasswordEdit(this);
    m_pRepeatPassword->lineEdit()->setPlaceholderText(tr("Input the password again"));
    connect(m_pRepeatPassword, &DPasswordEdit::textEdited,
            this, &DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength);
    connect(m_pRepeatPassword, &DPasswordEdit::editingFinished,
            this, &DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished);

    // 提示信息
    m_pPasswordHintLabel = new QLabel(tr("Hint"), this);
    m_pTips = new QLineEdit(this);
    m_pTips->setMaxLength(14);
    m_pTips->setPlaceholderText(tr("Password Hint"));

    // 其他解锁方式
    m_pOtherMethod = new QCheckBox(tr("Fingerprint"), this);
    m_pOtherMethod->setVisible(false);

    // 下一步按钮
    m_pNext = new QPushButton(tr("Next"), this);
    m_pNext->setMinimumWidth(450);
    m_pNext->setEnabled(false);
    connect(m_pNext, &QPushButton::clicked,
            this, &DFMVaultActiveSetUnlockMethodView::slotNextBtnClicked);

    // 长度
    m_pLengthLabel = new QLabel(tr("Length"), this);
    m_pLengthSlider = new QSlider(this);
    m_pLengthSlider->setOrientation(Qt::Horizontal);
    m_pLengthSlider->setRange(8, 24);
    connect(m_pLengthSlider, &QSlider::valueChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotLengthChanged);
    m_pLengthEdit = new QLineEdit(this);
    m_pLengthEdit->setMaximumWidth(60);
    m_pLengthEdit->setReadOnly(true);
    m_pLengthEdit->setText(tr("8Bit"));
    m_pStrengthLabel = new QLabel(tr("Week"), this);
    m_pStrengthLabel->setStyleSheet("color: #FF4500");
    m_pStrengthLabel->setMaximumWidth(50);
    m_pLengthLabel->setVisible(false);
    m_pLengthSlider->setVisible(false);
    m_pLengthEdit->setVisible(false);
    m_pStrengthLabel->setVisible(false);

    // 结果
    m_pResultLabel = new QLabel(tr("Result"), this);
    m_pResultEdit = new QLineEdit(tr(""), this);
    connect(m_pResultEdit, &QLineEdit::textChanged,
            this, &DFMVaultActiveSetUnlockMethodView::slotGenerateEditChanged);
    m_pResultLabel->setVisible(false);
    m_pResultEdit->setVisible(false);

    // 生成随即密码按钮
    m_pGenerateBtn = new QPushButton(tr("Generate"), this);
    connect(m_pGenerateBtn, &QPushButton::clicked,
            this, &DFMVaultActiveSetUnlockMethodView::slotGeneratePasswordBtnClicked);
    m_pGenerateBtn->setVisible(false);

    // 布局
    play = new QGridLayout();
    play->setMargin(1);
    play->addWidget(pLabel, 0, 0, 1, 6, Qt::AlignHCenter);

    play->addWidget(pTypeLabel, 1, 0, 1, 1, Qt::AlignLeft);
    play->addWidget(m_pTypeCombo, 1, 1, 1, 5);

    play->addWidget(m_pPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
    play->addWidget(m_pPassword, 2, 1, 1,5);

    play->addWidget(m_pRepeatPasswordLabel, 3, 0, 1, 1, Qt::AlignLeft);
    play->addWidget(m_pRepeatPassword, 3, 1, 1, 5);

    play->addWidget(m_pPasswordHintLabel, 4, 0, 1, 1, Qt::AlignLeft);
    play->addWidget(m_pTips, 4, 1, 1, 5);

//    play->addWidget(m_pOtherMethod, 5, 2, 1, 2, Qt::AlignHCenter);

    QSpacerItem *pSpacer = new QSpacerItem(1, 1, QSizePolicy::Minimum,
                                           QSizePolicy::Expanding);

    QVBoxLayout *play2 = new QVBoxLayout(this);
    play2->setMargin(1);
    play2->addLayout(play);
    play2->addSpacerItem(pSpacer);
    play2->addWidget(m_pNext, 0, Qt::AlignCenter);

    // 创建文件夹与目录
    if(!OperatorCenter::getInstance().createDirAndFile()) return;
}

void DFMVaultActiveSetUnlockMethodView::slotIsShowPassword()
{
    if(m_pPassword->echoMode() == QLineEdit::Password){
        m_pPassword->setEchoMode(QLineEdit::Normal);
    } else {
        m_pPassword->setEchoMode(QLineEdit::Password);
    }
}

void DFMVaultActiveSetUnlockMethodView::slotIshowRepeatPassword()
{
    if(m_pRepeatPassword->echoMode() == QLineEdit::Password){
        m_pRepeatPassword->setEchoMode(QLineEdit::Normal);
    } else {
        m_pRepeatPassword->setEchoMode(QLineEdit::Password);
    }
}

void DFMVaultActiveSetUnlockMethodView::slotPasswordEditFinished()
{
    bool ok = checkPassword(m_pPassword->text());
    if(!ok){
        m_pPassword->showAlertMessage(tr("At least 8 characters, and contain A-Z, a-z, 0-9, and symbols"));
    } else {
        if(checkInputInfo()){
            m_pNext->setEnabled(true);
        }
    }
}

void DFMVaultActiveSetUnlockMethodView::slotRepeatPasswordEditFinished()
{
    bool ok = checkRepeatPassword();
    if(!ok){
        m_pRepeatPassword->showAlertMessage(tr("Passwords do not match"));
    } else {
        if(checkInputInfo()){
            m_pNext->setEnabled(true);
        }
    }
}

void DFMVaultActiveSetUnlockMethodView::slotGenerateEditChanged(const QString &str)
{
    if(!checkPassword(str)){
        m_pNext->setEnabled(false);
    }else{
        m_pNext->setEnabled(true);
    }
}

void DFMVaultActiveSetUnlockMethodView::slotNextBtnClicked()
{
    QString strPassword = m_pPassword->text();
    QString strPasswordHint = m_pTips->text();
    if(OperatorCenter::getInstance().saveSaltAndClipher(strPassword, strPasswordHint)
            && OperatorCenter::getInstance().createKey(strPassword, USER_KEY_LENGTH))
        emit sigAccepted();
}

void DFMVaultActiveSetUnlockMethodView::slotTypeChanged(int index)
{
    if(index){  // 随机

        play->removeWidget(m_pPasswordLabel);
        play->removeWidget(m_pPassword);
        play->removeWidget(m_pRepeatPasswordLabel);
        play->removeWidget(m_pRepeatPassword);
//        play->removeWidget(m_pOtherMethod);

        m_pPasswordLabel->setVisible(false);
        m_pPassword->setVisible(false);
        m_pRepeatPasswordLabel->setVisible(false);
        m_pRepeatPassword->setVisible(false);
//        m_pOtherMethod->setVisible(false);

        play->addWidget(m_pLengthLabel, 2, 0, 1, 1, Qt::AlignLeft);
        play->addWidget(m_pLengthSlider, 2, 1, 1, 3);
        play->addWidget(m_pLengthEdit, 2, 4, 1, 1);
        play->addWidget(m_pStrengthLabel, 2, 5, 1, 1);
        play->addWidget(m_pResultLabel, 3, 0, 1, 1, Qt::AlignLeft);
        play->addWidget(m_pResultEdit, 3, 1, 1, 5);
        play->addWidget(m_pGenerateBtn, 5, 2, 1, 2, Qt::AlignHCenter);

        m_pLengthLabel->setVisible(true);
        m_pLengthSlider->setVisible(true);
        m_pLengthEdit->setVisible(true);
        m_pStrengthLabel->setVisible(true);
        m_pResultLabel->setVisible(true);
        m_pResultEdit->setVisible(true);
        m_pGenerateBtn->setVisible(true);

        // 检测密码的正确性
        slotGenerateEditChanged(m_pResultEdit->text());

    }else{  // 手动

        play->addWidget(m_pPasswordLabel, 2, 0, 1, 1, Qt::AlignLeft);
        play->addWidget(m_pPassword, 2, 1, 1, 5);
        play->addWidget(m_pRepeatPasswordLabel, 3, 0, 1, 1, Qt::AlignLeft);
        play->addWidget(m_pRepeatPassword, 3, 1, 1, 5);
//        play->addWidget(m_pOtherMethod, 5, 2, 1, 2, Qt::AlignHCenter);

        m_pPasswordLabel->setVisible(true);
        m_pPassword->setVisible(true);
        m_pRepeatPasswordLabel->setVisible(true);
        m_pRepeatPassword->setVisible(true);
//        m_pOtherMethod->setVisible(true);

        play->removeWidget(m_pLengthLabel);
        play->removeWidget(m_pLengthSlider);
        play->removeWidget(m_pLengthEdit);
        play->removeWidget(m_pStrengthLabel);
        play->removeWidget(m_pResultLabel);
        play->removeWidget(m_pResultEdit);
        play->removeWidget(m_pGenerateBtn);

        m_pLengthLabel->setVisible(false);
        m_pLengthSlider->setVisible(false);
        m_pLengthEdit->setVisible(false);
        m_pStrengthLabel->setVisible(false);
        m_pResultLabel->setVisible(false);
        m_pResultEdit->setVisible(false);
        m_pGenerateBtn->setVisible(false);

        // 检测密码正确性
        slotRepeatPasswordEditFinished();
    }
}

void DFMVaultActiveSetUnlockMethodView::slotLengthChanged(int length)
{
    m_pLengthEdit->setText(QString("%1%2").arg(length).arg(tr("Bit")));
    if(length > 7 && length < 12){
        m_pStrengthLabel->setText(tr("Week"));
        m_pStrengthLabel->setStyleSheet("color: #FF4500");
    }else if(length >= 12 && length < 19){
        m_pStrengthLabel->setText(tr("Medium"));
        m_pStrengthLabel->setStyleSheet("color: #EEC900");
    }else if(length >= 19 && length < 25){
        m_pStrengthLabel->setText(tr("Strong"));
        m_pStrengthLabel->setStyleSheet("color: #32CD32");
    }
}

void DFMVaultActiveSetUnlockMethodView::slotLimiPasswordLength(const QString &password)
{
    DPasswordEdit* pPasswordEdit = qobject_cast<DPasswordEdit*>(sender());
    if(password.length() > PASSWORD_LENGHT_MAX){
        pPasswordEdit->setText(password.mid(0, PASSWORD_LENGHT_MAX));
    }
}

void DFMVaultActiveSetUnlockMethodView::slotGeneratePasswordBtnClicked()
{
    QString strPassword = OperatorCenter::getInstance().autoGeneratePassword(m_pLengthSlider->value());
    m_pResultEdit->setText(strPassword);
}

bool DFMVaultActiveSetUnlockMethodView::checkPassword(const QString &password)
{
    QString strPassword = password;

//    QRegExp rx("^(?![^a-zA-Z]+$)(?!\\D+$)(?![a-zA-Z0-9]+$).{8,}$");
    QRegExp rx("^(?![^a-z]+$)(?![^A-Z]+$)(?!\\D+$)(?![a-zA-Z0-9]+$).{8,}$");
    QRegExpValidator v(rx);
    int pos = 0;
    QValidator::State res;
    res = v.validate(strPassword, pos);
    if(QValidator::Acceptable != res){
        return false;

    } else {
        return true;
    }
}

bool DFMVaultActiveSetUnlockMethodView::checkRepeatPassword()
{
    const QString &strRepeatPassword = m_pRepeatPassword->text();
    const QString &strPassword = m_pPassword->text();
    if(strRepeatPassword != strPassword)
    {
        return false;
    } else {
        return true;
    }
}

bool DFMVaultActiveSetUnlockMethodView::checkInputInfo()
{
    if(checkPassword(m_pPassword->text()) && checkRepeatPassword()){
        return true;
    } else {
        return false;
    }
}
