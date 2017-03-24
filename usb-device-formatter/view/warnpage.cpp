#include "warnpage.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

WarnPage::WarnPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void WarnPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QLabel* warnIconLabel = new QLabel(this);
    warnIconLabel->setPixmap(QPixmap(":/icoms/icons/info.png"));
    QLabel* warnTextLabel = new QLabel(this);
    warnTextLabel->setWordWrap(true);
    QString warnText = tr("Formating operation will clear all data from your disk, are you sure to continue? This option can not be restored.");
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
