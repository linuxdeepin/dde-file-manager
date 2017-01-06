#include "errorpage.h"
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>

ErrorPage::ErrorPage(QWidget *parent) : QFrame(parent)
{
    initUI();
}

void ErrorPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QLabel* warnIconLabel = new QLabel(this);
    warnIconLabel->setPixmap(QPixmap(":/icoms/icons/fail.png"));
    QLabel* warnTextLabel = new QLabel(this);
    QString warnText = tr("Format failed");
    warnTextLabel->setText(warnText);
    warnTextLabel->setWordWrap(true);

    mainLayout->addWidget(warnIconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(warnTextLabel, 0, Qt::AlignHCenter);
    setLayout(mainLayout);
}
