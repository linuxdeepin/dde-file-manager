#include "mainpage.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include "../partman/partitionmanager.h"
#include "../partman/readusagemanager.h"
#include "../partman/partition.h"
#include <QMetaEnum>
#include <QDebug>
#include <QProcess>
using namespace PartMan;

QString sizeString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

MainPage::MainPage(const QString& defautFormat, QWidget *parent) : QWidget(parent)
{
    m_defautlFormat = defautFormat;
    animator = new QVariantAnimation(this);
    animator->setDuration(100);
    initUI();
    initConnections();

    onCurrentSelectedTypeChanged(m_typeCombo->currentText());
}

void MainPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QIcon icon(":/icoms/icons/drive-removable-media-usb.svg");
    m_iconLabel = new QLabel(this);
    m_iconLabel->setPixmap(icon.pixmap(80, 80));
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);

    QVBoxLayout* storageProgressLayout = new QVBoxLayout;
    m_nameLabel = new QLabel(this);
    m_remainLabel = new QLabel(this);
    m_nameLabel->setObjectName("TagLabel");
    m_remainLabel->setObjectName("TagLabel");

    QHBoxLayout* shLayout = new QHBoxLayout;
    shLayout->addSpacing(35);
    shLayout->addWidget(m_nameLabel);
    shLayout->addStretch(1);
    shLayout->addWidget(m_remainLabel);
    shLayout->addSpacing(35);
    m_storageProgressBar = new ProgressLine(this);
    m_storageProgressBar->setMax(100);
    m_storageProgressBar->setMin(0);
    m_storageProgressBar->setValue(20);


    m_storageProgressBar->setFixedSize(qobject_cast<QWidget*>(parent())->width() - 70, 2);
    storageProgressLayout->addLayout(shLayout);
    storageProgressLayout->addWidget(m_storageProgressBar, 0, Qt::AlignHCenter);

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
    m_typeCombo = new DComboBox(this);
    m_typeCombo->setObjectName("TypeCombo");

    QMetaEnum metaEnum = QMetaEnum::fromType<FsType>();
    int index = 0;
    for(int i = 0; i < metaEnum.keyCount(); i++){
        QString key = metaEnum.key(i);
        m_fileFormat << key;
        if(m_defautlFormat == key.toLower()){
            index = i;
        }
    }

    m_typeCombo->addItems(m_fileFormat);
    m_typeCombo->setCurrentIndex(index);
    m_typeCombo->setFixedSize(160,22);

    QLabel* labelText = new QLabel(tr("Label"),this);
    labelText->setObjectName("TagLabel");
    m_labelLineEdit = new QLineEdit(this);
    m_labelLineEdit->setText(m_typeCombo->currentText());
    m_labelLineEdit->setFixedSize(160, 22);

    QCheckBox* fastFormatCheckBox = new QCheckBox(this);
    fastFormatCheckBox->setChecked(true);

    QLabel* fastFormatLabel = new QLabel(tr("Fast format"), this);
    fastFormatLabel->setObjectName("CheckBoxLabel");

    optGridLayout->addWidget(typeLable,0,0,Qt::AlignVCenter|Qt::AlignRight);
    optGridLayout->addWidget(m_typeCombo,0,1,Qt::AlignVCenter|Qt::AlignLeft);

    optGridLayout->addWidget(labelText, 1, 0, Qt::AlignVCenter|Qt::AlignRight);
    optGridLayout->addWidget(m_labelLineEdit, 1, 1, Qt::AlignVCenter|Qt::AlignLeft);

    optGridLayout->addWidget(fastFormatCheckBox,2,0,Qt::AlignVCenter|Qt::AlignRight);
    optGridLayout->addWidget(fastFormatLabel,2,1,Qt::AlignVCenter|Qt::AlignLeft);

    for(int i = 0; i < optGridLayout->rowCount(); i++){
        optGridLayout->setRowMinimumHeight(i, 30);
    }
    mainLayout->addLayout(optGridLayout);

    QString warmMsg = tr("This operation will clear all data from your device.");
    m_warnLabel = new QLabel(this);
    m_warnLabel->setText(warmMsg);
    m_warnLabel->setObjectName("WarnMsg");
    m_warnLabel->setWordWrap(true);
    m_warnLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(m_warnLabel, 0, Qt::AlignHCenter);
    setLayout(mainLayout);
}

QString MainPage::selectedFormat()
{
    return m_typeCombo->currentText();
}

void MainPage::initConnections()
{
    connect(m_typeCombo, &QComboBox::currentTextChanged, this, &MainPage::onCurrentSelectedTypeChanged);
}

QString MainPage::getLabel()
{
    return m_labelLineEdit->text();
}

void MainPage::onCurrentSelectedTypeChanged(const QString &type)
{
    m_labelLineEdit->setText(type);
    m_maxLabelNameLength = PartMan::PartitionManager::getMaxNameLengthByTypeString(type);
    m_labelLineEdit->setMaxLength(m_maxLabelNameLength);
}

void MainPage::resizeEvent(QResizeEvent *event)
{
    m_warnLabel->setFixedWidth(this->width() -80);
    QWidget::resizeEvent(event);
}

int MainPage::getMaxLabelNameLength() const
{
    return m_maxLabelNameLength;
}

void MainPage::setMaxLabelNameLength(int maxLabelNameLength)
{
    m_maxLabelNameLength = maxLabelNameLength;
}

QString MainPage::getTargetPath() const
{
    return m_targetPath;
}

void MainPage::setTargetPath(const QString &targetPath)
{
    qlonglong total, free;
    PartMan::ReadUsageManager readUsageManager;
    readUsageManager.readUsage(targetPath, free, total);

    m_storageProgressBar->setMax(total);
    m_storageProgressBar->setValue(total - free);
    m_remainLabel->setText(QString("%1/ %2").arg(formatSize(total - free), formatSize(total)));
    QString deviceName = Partition::getPartitionByDevicePath(targetPath).label();
    QFontMetrics fm(QFont("",10));
    m_nameLabel->setText(fm.elidedText(deviceName, Qt::ElideRight, 100  ));

    qint64 max = 4;
    max = max * 1024;
    max = max * 1024;
    max = max * 1024;

    //If removable disk size is larger than 4GB,Just support fat32 and ntfs type
    if(total > max)
        m_typeCombo->removeItem(0);
}

QString MainPage::formatSize(const qint64 &num)
{
    QString total;
    const qint64 kb = 1024;
    const qint64 mb = 1024 * kb;
    const qint64 gb = 1024 * mb;
    const qint64 tb = 1024 * gb;

    if ( num >= tb ) {
        total = QString( "%1 TB" ).arg( sizeString(QString::number( qRound64(qreal( num ) / tb), 'f', 1 )) );
    } else if( num >= gb ) {
        total = QString( "%1 GB" ).arg( sizeString(QString::number( qRound64(qreal( num ) / gb), 'f', 1 )) );
    } else if( num >= mb ) {
        total = QString( "%1 MB" ).arg( sizeString(QString::number( qRound64(qreal( num ) / mb), 'f', 1 )) );
    } else if( num >= kb ) {
        total = QString( "%1 KB" ).arg( sizeString(QString::number( qRound64(qreal( num ) / kb),'f',1 )) );
    } else {
        total = QString( "%1 B" ).arg( num );
    }

    return total;
}

QString MainPage::getSelectedFs() const
{
    return m_typeCombo->currentText();
}

