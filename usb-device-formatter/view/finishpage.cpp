#include "finishpage.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

FinishPage::FinishPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void FinishPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QLabel* warnIconLabel = new QLabel(this);
    warnIconLabel->setPixmap(QPixmap(":/icoms/icons/success.png"));
    QLabel* warnTextLabel = new QLabel(this);
    QString warnText = tr("Format successfully");
    warnTextLabel->setText(warnText);
    warnTextLabel->setWordWrap(true);
    warnTextLabel->setObjectName("StatusLabel");
    warnTextLabel->setAlignment(Qt::AlignHCenter);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(warnIconLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(warnTextLabel, 0, Qt::AlignHCenter);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
    setContentsMargins(20, 0, 20, 0);
}
