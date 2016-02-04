#include "dbasedialog.h"
#include "widgets/util.h"
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QDesktopWidget>


DBaseDialog::DBaseDialog(QWidget *parent):DMovabelDialog(parent)
{
    m_closeButton = new QPushButton(this);
    m_closeButton->setObjectName("CloseButton");
    m_closeButton->setFixedSize(25, 25);
    m_closeButton->setAttribute(Qt::WA_NoMousePropagation);
    m_messageLabelMaxWidth = qApp->desktop()->availableGeometry().width() / 2 -
            100 - 2 * m_closeButton->width();
}

void DBaseDialog::initUI(const QString &icon,
                         const QString &message,
                         const QString &tipMessage,
                         const QStringList &buttonKeys,
                         const QStringList &buttonTexts){
    m_icon = icon;
    m_message = message;
    m_tipMessage = tipMessage;
    m_buttonKeys = buttonKeys;
    m_buttonTexts = buttonTexts;

    QFrame* contentFrame = new QFrame;
    contentFrame->setObjectName("ContentFrame");

    m_iconLabel = new QLabel;
    m_iconLabel->setFixedSize(48, 48);
    setIcon(m_icon);

    m_messageLabel = new QLabel;
    m_messageLabel->setObjectName("MessageLabel");
    setMessage(m_message);

    m_tipMessageLabel = new QLabel;
    m_tipMessageLabel->setObjectName("TipMessageLabel");
    setTipMessage(m_tipMessage);

    m_buttonGroup = new QButtonGroup;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    foreach (QString label, m_buttonKeys) {
        int index = m_buttonKeys.indexOf(label);
        QPushButton* button = new QPushButton(label);
        button->setObjectName("ActionButton");
        button->setAttribute(Qt::WA_NoMousePropagation);
        button->setFixedHeight(28);
        m_buttonGroup->addButton(button, index);
        buttonLayout->addWidget(button);
        if (index < m_buttonKeys.length() - 1){
            QLabel* label = new QLabel;
            label->setObjectName("VLine");
            label->setFixedWidth(1);
            buttonLayout->addWidget(label);
        }
    }

    setButtonTexts(m_buttonTexts);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout* iconLayout = new QVBoxLayout;
    iconLayout->addWidget(m_iconLabel, Qt::AlignCenter);
    iconLayout->setContentsMargins(26, 22, 26, 22);

    m_messageLayout = new QVBoxLayout;
    m_messageLayout->addWidget(m_messageLabel);
    m_messageLayout->addWidget(m_tipMessageLabel);
    m_messageLayout->addStretch();
    m_messageLayout->setContentsMargins(5, m_closeButton->height(), m_closeButton->width(), 0);

    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->addLayout(iconLayout);
    topLayout->addLayout(m_messageLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    contentFrame->setLayout(mainLayout);

    QHBoxLayout* contentlayout = new QHBoxLayout;
    contentlayout->addWidget(contentFrame);
    contentlayout->setContentsMargins(5, 5, 5, 5);
    setLayout(contentlayout);

    resize(m_defaultWidth, m_defaultHeight);
    setStyleSheet(getQssFromFile(":/qss/dialogs/qss/dialogs.qss"));
    initConnect();
}


void DBaseDialog::initConnect(){
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(handleButtonsClicked(int)));
    connect(m_closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

QButtonGroup* DBaseDialog::getButtonsGroup(){
    return m_buttonGroup;
}

QVBoxLayout* DBaseDialog::getMessageLayout(){
    return m_messageLayout;
}

QString DBaseDialog::getIcon(){
    return m_icon;
}

void DBaseDialog::setIcon(const QString &icon){
    m_icon = icon;
    m_iconLabel->setPixmap(QPixmap(m_icon).scaled(m_iconLabel->size()));
}

QString DBaseDialog::getMessage(){
    return m_message;
}

void DBaseDialog::setMessage(const QString &message){
    m_message = message;

    QFontMetrics fm = fontMetrics();
    if (fm.width(m_message) > m_messageLabelMaxWidth){
        QString text = fm.elidedText(m_message, Qt::ElideRight, m_messageLabelMaxWidth);
        m_messageLabel->setText(text);
    }else{
        m_messageLabel->setText(m_message);
    }
    m_messageLabel->resize(fm.width(m_messageLabel->text()), height());
}

QString DBaseDialog::getTipMessage(){
    return m_tipMessage;
}

void DBaseDialog::setTipMessage(const QString &tipMessage){
    m_tipMessage = tipMessage;

    QFontMetrics fm = fontMetrics();
    if (fm.width(m_tipMessage) > m_messageLabelMaxWidth){
        QString text = fm.elidedText(m_tipMessage, Qt::ElideRight, m_messageLabelMaxWidth);
        m_tipMessageLabel->setText(text);
    }else{
        m_tipMessageLabel->setText(m_tipMessage);
    }
    m_tipMessageLabel->resize(fm.width(m_messageLabel->text()), height());
}

QStringList DBaseDialog::getButtons(){
    return m_buttonKeys;
}

void DBaseDialog::setButtons(const QStringList &buttons){
    m_buttonKeys = buttons;
}

QStringList DBaseDialog::getButtonTexts(){
    return m_buttonTexts;
}

void DBaseDialog::setButtonTexts(const QStringList& buttonTexts){
    if (buttonTexts.length() != m_buttonKeys.length()){
        return;
    }
    m_buttonTexts = buttonTexts;
    for (int i = 0; i < m_buttonGroup->buttons().length(); i++) {
        QPushButton* button = reinterpret_cast<QPushButton*>(m_buttonGroup->buttons().at(i));
        button->setText(m_buttonTexts.at(i));
    }
}

void DBaseDialog::handleButtonsClicked(int id){
    close();
    int index = m_buttonGroup->buttons().indexOf(m_buttonGroup->button(id));
    if (m_buttonKeys.length() > index){
        QString key = m_buttonKeys.at(index);
        emit buttonClicked(key);
        emit buttonClicked(index);
    }
}

void DBaseDialog::handleKeyEnter(){

}

void DBaseDialog::closeEvent(QCloseEvent *event){
    emit aboutToClose();
    DMovabelDialog::closeEvent(event);
    emit closed();
}


void DBaseDialog::resizeEvent(QResizeEvent *event){
    m_closeButton->move(width() - m_closeButton->width() - 4, 4);
    m_closeButton->raise();
    moveCenter();
    DMovabelDialog::resizeEvent(event);
}

DBaseDialog::~DBaseDialog()
{

}

