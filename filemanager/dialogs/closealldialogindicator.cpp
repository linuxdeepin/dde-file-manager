#include "closealldialogindicator.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QDesktopWidget>
#include <QApplication>

CloseAllDialogIndicator::CloseAllDialogIndicator(QWidget *parent) : BaseDialog(parent)
{
    initUI();
    initConnect();
}

CloseAllDialogIndicator::~CloseAllDialogIndicator()
{

}

void CloseAllDialogIndicator::initUI()
{
    resize(QSize(400, 30));


    m_messageLabel = new QLabel(this);
    m_closeButton = new QPushButton(tr("close all"), this);
    m_closeButton->setObjectName("AllCloseButton");

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_messageLabel, Qt::AlignCenter);
    mainLayout->addSpacing(50);
    mainLayout->addWidget(m_closeButton, Qt::AlignRight);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    QRect screenGeometry = qApp->desktop()->screenGeometry();

    move((screenGeometry.width() - width()) / 2, screenGeometry.height() - height());

    setTotalMessage(0, 0);
}

void CloseAllDialogIndicator::initConnect()
{
    connect(m_closeButton, &QPushButton::clicked, this, &CloseAllDialogIndicator::allClosed);
}

void CloseAllDialogIndicator::setTotalMessage(int size, int count)
{
    QString message = tr("Total size %1, Total files count %2").arg(QString::number(size), QString::number(count));
    m_messageLabel->setText(message);
}

