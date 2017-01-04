#include "mainpage.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>

MainPage::MainPage(QWidget *parent) : QWidget(parent)
{
    initUI();
}

void MainPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QIcon icon(":/icoms/icons/drive-removable-media-usb.svg");
    m_iconLabel = new QLabel(this);
    m_iconLabel->setPixmap(icon.pixmap(80, 80));
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);

    QVBoxLayout* storageProgressLayout = new QVBoxLayout;
    QLabel* docLabel = new QLabel(tr("My Documents"), this);
    QLabel* remainLabel = new QLabel("28.5G/32G", this);
    docLabel->setObjectName("TagLabel");
    remainLabel->setObjectName("TagLabel");

    QHBoxLayout* shLayout = new QHBoxLayout;
    shLayout->addSpacing(35);
    shLayout->addWidget(docLabel);
    shLayout->addStretch(1);
    shLayout->addWidget(remainLabel);
    shLayout->addSpacing(35);
    m_sorageProgressBar = new ProgressLine(this);
    m_sorageProgressBar->setMax(32);
    m_sorageProgressBar->setMin(0);
    m_sorageProgressBar->setValue(28);
    m_sorageProgressBar->setFixedSize(qobject_cast<QWidget*>(parent())->width() - 70, 2);
    storageProgressLayout->addLayout(shLayout);
    storageProgressLayout->addWidget(m_sorageProgressBar, 0, Qt::AlignHCenter);

    mainLayout->addLayout(storageProgressLayout);
    mainLayout->addSpacing(15);
    QLabel* line = new QLabel(this);
    line->setObjectName("Line");
    line->setFixedSize(qobject_cast<QWidget*>(parent())->width() - 20, 2);
    mainLayout->addWidget(line, 0, Qt::AlignHCenter);

    mainLayout->addSpacing(16);
    QGridLayout* optGridLayout = new QGridLayout;
    optGridLayout->setColumnMinimumWidth(1, 230);
    optGridLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* typeLable =new  QLabel(tr("Type"), this);
    typeLable->setObjectName("TagLabel");
    typeLable->setFont(QFont("",10));
    m_typeCombo = new QComboBox(this);

    m_fileFormat << "ext4" << "ext3" << "fat" << "nfs";
    m_typeCombo->addItems(m_fileFormat);

    m_typeCombo->setFixedSize(160,22);
    optGridLayout->addWidget(typeLable,0,0,Qt::AlignVCenter|Qt::AlignRight);
    optGridLayout->addWidget(m_typeCombo,0,1,Qt::AlignVCenter|Qt::AlignLeft);

    QCheckBox* fastFormatCheckBox = new QCheckBox(this);
    QLabel* fastFormatLabel = new QLabel(tr("Fast format"), this);
    optGridLayout->addWidget(fastFormatCheckBox,1,0,Qt::AlignVCenter|Qt::AlignRight);
    optGridLayout->addWidget(fastFormatLabel,1,1,Qt::AlignVCenter|Qt::AlignLeft);

    for(int i = 0; i < optGridLayout->rowCount(); i++){
        optGridLayout->setRowMinimumHeight(i, 30);
    }
    mainLayout->addLayout(optGridLayout);

    QString warmMsg = tr("This operation will clear all datas from your device.");
    QLabel* warmLabel = new QLabel(this);
    warmLabel->setText(warmMsg);
    warmLabel->setWordWrap(true);
    mainLayout->addWidget(warmLabel, 0, Qt::AlignHCenter);
    setLayout(mainLayout);
}
