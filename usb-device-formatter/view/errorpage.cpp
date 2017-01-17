#include "errorpage.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QResizeEvent>

ErrorPage::ErrorPage(const QString &errorStr, QWidget *parent):
    QFrame(parent)
{
    m_errorText = errorStr;
    initUI();
}

void ErrorPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QLabel* warnIconLabel = new QLabel(this);
    warnIconLabel->setPixmap(QPixmap(":/icoms/icons/fail.png"));
    m_errorLabel = new QLabel(this);
    m_errorLabel->setText(m_errorText);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setAlignment(Qt::AlignHCenter);

    mainLayout->addWidget(warnIconLabel, 0, Qt::AlignHCenter);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_errorLabel, 0, Qt::AlignHCenter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void ErrorPage::setErrorMsg(const QString &errorMsg)
{
    m_errorLabel->setText(errorMsg);
}

void ErrorPage::resizeEvent(QResizeEvent *event)
{
    m_errorLabel->setFixedWidth(event->size().width() - 80);
}
