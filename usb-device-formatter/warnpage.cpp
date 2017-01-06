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
    QString warnText = tr("Formating operation will clear all datas from your disk, are you sure to continue?This option can not be restored.");
    warnTextLabel->setText(warnText);
    warnTextLabel->setWordWrap(true);

    mainLayout->addWidget(warnIconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(warnTextLabel, 0, Qt::AlignHCenter);
    setLayout(mainLayout);
}
