#include "mainpage.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include "../partman/partitionmanager.h"
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
    initUI();
    initConnections();
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
    m_remainLabel = new QLabel(this);
    docLabel->setObjectName("TagLabel");
    m_remainLabel->setObjectName("TagLabel");

    QHBoxLayout* shLayout = new QHBoxLayout;
    shLayout->addSpacing(35);
    shLayout->addWidget(docLabel);
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
    typeLable->setFont(QFont("",10));
    m_typeCombo = new QComboBox(this);
    m_typeCombo->setObjectName("TypeCombo");

    QMetaEnum metaEnum = QMetaEnum::fromType<PartitionManager::FsType>();
    for(int i = 0; i < metaEnum.keyCount(); i++){
        QString key = metaEnum.key(i);
        m_fileFormat << key.toLower();
    }
    m_fileFormat.removeLast();

    m_typeCombo->addItems(m_fileFormat);
    int index = m_fileFormat.indexOf(m_defautlFormat);
    if(index >= 0)
        m_typeCombo->setCurrentIndex(index);

    m_typeCombo->setFixedSize(160,22);

    QLabel* labelText = new QLabel(tr("Label"),this);
    labelText->setObjectName("TagLabel");
    m_labelLineEdit = new QLineEdit(this);
    m_labelLineEdit->setText(m_defautlFormat);
    m_labelLineEdit->setFixedSize(160, 22);

    QCheckBox* fastFormatCheckBox = new QCheckBox(this);
    QLabel* fastFormatLabel = new QLabel(tr("Fast format"), this);

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

    QString warmMsg = tr("This operation will clear all datas from your device.");
    QLabel* warmLabel = new QLabel(this);
    warmLabel->setText(warmMsg);
    warmLabel->setWordWrap(true);
    mainLayout->addWidget(warmLabel, 0, Qt::AlignHCenter);
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
}

QString MainPage::getTargetPath() const
{
    return m_targetPath;
}

void MainPage::setTargetPath(const QString &targetPath)
{
    m_targetPath = targetPath;
    QProcess p;
    QString cmd = "df";
    QStringList args;
    args << "-BK" << "--output=avail,used" << m_targetPath;
    p.start(cmd, args);
    p.waitForFinished();
    p.readLine();
    QString result = p.readLine();
    result.chop(1);
    QStringList datas = result.split(" ");

    for(int i = 0; i < datas.size(); i++){
        QString data = datas.at(i);
        if(data.isEmpty()){
            datas.removeAt(i);
            i--;
            continue;
        }
        data.chop(1);
        datas.replace(i,data);
    }
    const qint64 max = QString(datas.at(0)).toLongLong();
    const qint64 used = QString(datas.at(1)).toLongLong();
    qDebug () << datas << "targetPath:" << targetPath;
    m_storageProgressBar->setMax(max);
    m_storageProgressBar->setValue(used);
    m_remainLabel->setText(tr("%1/ %2").arg(formatSize(used), formatSize(max)));
}

QString MainPage::formatSize(const qint64 &num)
{
    QString total;
    const qint64 kb = 1;
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

