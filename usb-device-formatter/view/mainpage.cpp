/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainpage.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCheckBox>
#include <QMetaEnum>
#include <QDebug>
#include <QProcess>

#include <DColoredProgressBar>

#include "utils/fsutils.h"
#include "utils/udisksutils.h"

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

    onCurrentSelectedTypeChanged(m_typeCombo->currentText());
}

void MainPage::initUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout;
    m_iconLabel = new QLabel(this);
    m_iconLabel->setPixmap(QIcon::fromTheme("drive-removable-media-usb").pixmap(80, 80));
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
    DTK_WIDGET_NAMESPACE::DColoredProgressBar *stpb;
    stpb = new DTK_WIDGET_NAMESPACE::DColoredProgressBar();
    QLinearGradient lg(0, 0.5, 1, 0.5);
    lg.setCoordinateMode(QGradient::CoordinateMode::ObjectBoundingMode);

    lg.setStops({{0, 0xFF0080FF}, {0.72, 0xFF0397FE}, {1, 0xFF06BEFD}});
    stpb->addThreshold(0, lg);

    lg.setStops({{0, 0xFFFFAE00}, {0.72, 0xFFFFD007}, {1, 0xFFF6FF0D}});
    stpb->addThreshold(7000, lg);

    lg.setStops({{0, 0xFFFF0000}, {0.72, 0xFFFF237A}, {1, 0xFFFF9393}});
    stpb->addThreshold(9000, lg);

    stpb->setMaximum(10000);
    stpb->setTextVisible(false);

    stpb->setFixedSize(qobject_cast<QWidget*>(parent())->width() - 70, 8);
    m_storageProgressBar = stpb;
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
    m_typeCombo = new QComboBox(this);
    m_typeCombo->setObjectName("TypeCombo");

    int index = 0, i = 0;
    for(auto &fs : FsUtils::supportedFilesystems()){
        m_fileFormat << fs;
        if(m_defautlFormat == fs.toLower()){
            index = i;
        }
        ++i;
    }

    m_typeCombo->addItems(m_fileFormat);
    m_typeCombo->setCurrentIndex(index);
    m_typeCombo->setFixedWidth(160);

    QLabel* labelText = new QLabel(tr("Label"),this);
    labelText->setObjectName("TagLabel");
    m_labelLineEdit = new QLineEdit(this);
    m_labelLineEdit->setText(m_typeCombo->currentText());
    m_labelLineEdit->setFixedWidth(160);

    m_quickfmt = new QCheckBox(this);
    m_quickfmt->setChecked(true);

    QLabel* fastFormatLabel = new QLabel(tr("Quick Format"), this);
    fastFormatLabel->setObjectName("CheckBoxLabel");

    optGridLayout->addWidget(typeLable,0,0,Qt::AlignVCenter|Qt::AlignRight);
    optGridLayout->addWidget(m_typeCombo,0,1,Qt::AlignVCenter|Qt::AlignLeft);

    optGridLayout->addWidget(labelText, 1, 0, Qt::AlignVCenter|Qt::AlignRight);
    optGridLayout->addWidget(m_labelLineEdit, 1, 1, Qt::AlignVCenter|Qt::AlignLeft);

    optGridLayout->addWidget(m_quickfmt,2,0,Qt::AlignVCenter|Qt::AlignRight);
    optGridLayout->addWidget(fastFormatLabel,2,1,Qt::AlignVCenter|Qt::AlignLeft);

    for(int i = 0; i < optGridLayout->rowCount(); i++){
        optGridLayout->setRowMinimumHeight(i, 30);
    }
    mainLayout->addLayout(optGridLayout);

    QString warmMsg = tr("Formatting will erase all data on the disk.");
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
    m_maxLabelNameLength = FsUtils::maxLabelLength(type);
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
    qlonglong total, used;
    UDisksBlock blk(targetPath);
    total = blk.sizeTotal();
    used =  blk.sizeUsed();

    if (~used) {
        m_storageProgressBar->setValue(total ? int(10000. * used / total) : 0);
        m_remainLabel->setText(QString("%1 / %2").arg(formatSize(used), formatSize(total)));
    } else {
        m_storageProgressBar->setValue(0);
        m_remainLabel->setText(formatSize(total));
    }
    QString deviceName = blk.displayName();
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

bool MainPage::shouldErase() const
{
    return !m_quickfmt->isChecked();
}

