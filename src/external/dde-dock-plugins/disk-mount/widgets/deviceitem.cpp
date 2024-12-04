// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
    if (event->button() != Qt::LeftButton)
        return;
    openDevice();
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
    setFixedSize(kDockPluginWidth, kDeviceItemHeight);

    QLabel *labName = new QLabel(data.displayName, this);
    labName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    labName->setTextFormat(Qt::PlainText);
    setTextFont(labName, 14, QFont::Medium);
    setTextColor(labName, DGuiApplicationHelper::instance()->themeType(), 0.8);

    sizeLabel = new QLabel(this);
    setTextFont(sizeLabel, 12, QFont::Normal);
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
    devInfoLay->addWidget(labName);
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
                setTextColor(labName, type, 0.8);
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

void DeviceItem::setTextColor(QWidget *obj, int themeType, double alpha)
{
    Q_ASSERT(obj);
    QPalette pal = obj->palette();
    int colorVal = (themeType == DGuiApplicationHelper::DarkType);
    pal.setColor(QPalette::WindowText, QColor::fromRgbF(colorVal, colorVal, colorVal, alpha));
    obj->setPalette(pal);
}

void DeviceItem::setTextFont(QWidget *widget, int pixelSize, int weight)
{
    Q_ASSERT(widget);
    QFont f = widget->font();
    f.setPixelSize(pixelSize);
    f.setWeight(static_cast<QFont::Weight>(weight));
    widget->setFont(f);
}
