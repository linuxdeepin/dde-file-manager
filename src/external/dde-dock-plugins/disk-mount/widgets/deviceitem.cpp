// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceitem.h"
#include "utils/dockutils.h"

#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QLoggingCategory>

#include <DGuiApplicationHelper>
#include <DIconButton>
#include <DDesktopServices>
#include <DFontSizeManager>

Q_DECLARE_LOGGING_CATEGORY(logAppDock)

using namespace Dtk::Gui;
using namespace Dtk::Widget;

DeviceItem::DeviceItem(const DockItemData &item, QWidget *parent)
    : QFrame(parent), data(item)
{
    initUI();
}

QFrame *DeviceItem::createSeparateLine(int width)
{
    QFrame *f = new QFrame();
    f->setLineWidth(width);
    f->setFixedHeight(width);
    f->setFrameStyle(QFrame::HLine);
    f->setFrameShadow(QFrame::Plain);
    return f;
}

void DeviceItem::mouseReleaseEvent(QMouseEvent *event)
{
    QFrame::mouseReleaseEvent(event);
    if (!common_utils::isIntegratedByFilemanager())
        return;
    if (event->button() != Qt::LeftButton)
        return;
    openDevice();
}

void DeviceItem::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);
    updateDeviceName();
}

void DeviceItem::showEvent(QShowEvent *e)
{
    updateDeviceName();
    QFrame::showEvent(e);
}

void DeviceItem::updateUsage(quint64 usedSize)
{
    if (usedSize > data.totalSize)
        usedSize = data.totalSize;
    data.usedSize = usedSize;

    sizeLabel->setText(QString("%1 / %2")
                               .arg(size_format::formatDiskSize(data.usedSize))
                               .arg(size_format::formatDiskSize(data.totalSize)));
    int progress = 100 * (1.0 * data.usedSize / data.totalSize);
    sizeProgress->setValue(progress);
}

void DeviceItem::initUI()
{
    setFixedWidth(kDockPluginWidth);

    nameLabel = new QLabel(data.displayName, this);
    nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    nameLabel->setTextFormat(Qt::PlainText);
    nameLabel->setWordWrap(true);
    DFontSizeManager::instance()->bind(nameLabel, DFontSizeManager::T5, QFont::Medium);
    setTextColor(nameLabel, DGuiApplicationHelper::instance()->themeType(), 0.8);

    sizeLabel = new QLabel(this);
    DFontSizeManager::instance()->bind(sizeLabel, DFontSizeManager::T7, QFont::Normal);
    setTextColor(sizeLabel, DGuiApplicationHelper::instance()->themeType(), 0.6);

    sizeProgress = new QProgressBar(this);
    sizeProgress->setTextVisible(false);
    sizeProgress->setFixedHeight(2);
    sizeProgress->setMinimum(0);
    sizeProgress->setMaximum(100);

    DIconButton *ejectBtn = new DIconButton(this);
    ejectBtn->setFixedSize(20, 20);
    ejectBtn->setIconSize({ 20, 20 });
    ejectBtn->setIcon(QIcon::fromTheme("dfm_dock_unmount"));

    QPushButton *deviceIcon = new QPushButton(this);
    deviceIcon->setFlat(true);
    deviceIcon->setIcon(QIcon::fromTheme(data.iconName));
    deviceIcon->setIconSize({ 48, 48 });
    deviceIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    deviceIcon->setStyleSheet("padding: 0;");

    QVBoxLayout *devIconLay = new QVBoxLayout();
    devIconLay->setContentsMargins(10, 8, 0, 8);
    devIconLay->setSpacing(0);
    devIconLay->addWidget(deviceIcon);

    QVBoxLayout *devInfoLay = new QVBoxLayout();
    devInfoLay->setSpacing(2);
    devInfoLay->setContentsMargins(10, 11, 0, 15);
    devInfoLay->addWidget(nameLabel);
    devInfoLay->addWidget(sizeLabel);
    QWidget *space = new QWidget(this);
    space->setFixedHeight(2);
    devInfoLay->addWidget(space);
    devInfoLay->addWidget(sizeProgress);

    QVBoxLayout *ejectBtnLay = new QVBoxLayout();
    ejectBtnLay->setContentsMargins(19, 22, 16, 22);
    ejectBtnLay->addWidget(ejectBtn);

    QHBoxLayout *itemLay = new QHBoxLayout();
    itemLay->setContentsMargins(10, 8, 8, 12);
    itemLay->setContentsMargins(0, 0, 0, 0);
    itemLay->setSpacing(0);
    itemLay->addLayout(devIconLay);
    itemLay->addLayout(devInfoLay);
    itemLay->addLayout(ejectBtnLay);

    QVBoxLayout *mainLay = new QVBoxLayout();
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    auto line = createSeparateLine(1);
    line->setParent(this);
    mainLay->addWidget(line);
    mainLay->addLayout(itemLay);
    setLayout(mainLay);

    connect(ejectBtn, &DIconButton::clicked,
            this, [this] { Q_EMIT requestEject(data.backingID); });
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, [=](auto type) {
                setTextColor(nameLabel, type, 0.8);
                setTextColor(sizeLabel, type, 0.6);
            });

    updateUsage(data.usedSize);
}

void DeviceItem::openDevice()
{
    qCInfo(logAppDock) << "about to open" << data.targetUrl << data.targetFileUrl;
    if (QStandardPaths::findExecutable("dde-file-manager").isEmpty())
        DDesktopServices::showFolder(data.targetFileUrl);
    else
        DDesktopServices::showFolder(data.targetUrl);
}

void DeviceItem::updateDeviceName()
{
    static QStringList schemeList { "ftp", "sftp", "nfs", "dav", "davs" };

    QString devName = data.displayName;
    if (data.isProtocolDevice) {
        QString host, share, alias;
        int port;
        if (smb_utils::parseSmbInfo(data.targetFileUrl.toString(), &host, &share, &port)) {
            alias = device_utils::protocolDeviceAlias("smb", host);
        } else if (std::any_of(schemeList.cbegin(), schemeList.cend(),
                               [this](const QString &scheme) {
                                   return data.id.startsWith(scheme);
                               })) {
            QUrl url(data.id);
            host = url.host();
            alias = device_utils::protocolDeviceAlias(url.scheme(), url.host());
        }
        if (!alias.isEmpty())
            devName.replace(host, alias);
    }

    devName = nameLabel->fontMetrics().elidedText(devName, Qt::ElideMiddle, nameLabel->width());
    nameLabel->setText(devName);
}

void DeviceItem::setTextColor(QWidget *obj, int themeType, double alpha)
{
    Q_ASSERT(obj);
    QPalette pal = obj->palette();
    int colorVal = (themeType == DGuiApplicationHelper::DarkType);
    pal.setColor(QPalette::WindowText, QColor::fromRgbF(colorVal, colorVal, colorVal, alpha));
    obj->setPalette(pal);
}
