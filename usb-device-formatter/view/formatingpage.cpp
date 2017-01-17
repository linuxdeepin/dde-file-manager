#include "formatingpage.h"

#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>

FormatingPage::FormatingPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void FormatingPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    m_progressBox = new ProgressBox(this);
    m_progressBox->setFixedSize(100, 100);
    QLabel* formatingLabel = new QLabel(this);
    QString formatingText = tr("Formating,please wait for a minute...");
    formatingLabel->setText(formatingText);
    formatingLabel->setObjectName("StatusLabel");

    connect(m_progressBox, &ProgressBox::finished, this, &FormatingPage::finished);

    mainLayout->addStretch(1);
    mainLayout->addWidget(m_progressBox, 0 , Qt::AlignHCenter);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(formatingLabel, 0 , Qt::AlignHCenter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void FormatingPage::animateToFinish(const bool &result)
{
    m_progressBox->finishedTask(result);
}

void FormatingPage::startAnimate()
{
    m_progressBox->setValue(0);
    m_progressBox->startTask();
}
