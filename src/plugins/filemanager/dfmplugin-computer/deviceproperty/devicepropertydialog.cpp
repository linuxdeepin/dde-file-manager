/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#include "devicepropertydialog.h"
#include "dfm-base/utils/universalutils.h"

#include <DDrawer>
#include <denhancedwidget.h>
#include <DArrowLineDrawer>
#include <DApplicationHelper>

#include <QKeyEvent>

const static int kArrowExpandSpacing = 10;

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPCOMPUTER_USE_NAMESPACE
DevicePropertyDialog::DevicePropertyDialog(QWidget *parent)
    : DDialog(parent)
{
    iniUI();
    this->setAttribute(Qt::WA_DeleteOnClose, true);
}

DevicePropertyDialog::~DevicePropertyDialog()
{
}

void DevicePropertyDialog::iniUI()
{
    deviceIcon = new DLabel(this);
    deviceIcon->setFixedHeight(128);

    deviceName = new DLabel(this);
    deviceName->setAlignment(Qt::AlignCenter);

    QFrame *basicInfoFrame = new QFrame(this);

    basicInfo = new KeyValueLabel(this);
    basicInfo->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7);
    devicesProgressBar = new DColoredProgressBar();
    devicesProgressBar->addThreshold(0, QColor(0xFF0080FF));
    devicesProgressBar->addThreshold(7000, QColor(0xFFFFAE00));
    devicesProgressBar->addThreshold(9000, QColor(0xFFFF0000));
    devicesProgressBar->setMaximumHeight(8);
    devicesProgressBar->setTextVisible(false);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setMargin(0);
    vlayout->setContentsMargins(12, 8, 12, 8);
    vlayout->addWidget(basicInfo);
    vlayout->addWidget(devicesProgressBar);
    basicInfoFrame->setLayout(vlayout);

    new DFMRoundBackground(basicInfoFrame, 8);

    QVBoxLayout *vlayout1 = new QVBoxLayout();
    vlayout1->setMargin(0);
    vlayout1->setSpacing(0);
    vlayout1->addWidget(deviceIcon, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout1->addWidget(deviceName, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout1->addWidget(basicInfoFrame);

    QFrame *frame = new QFrame(this);
    frame->setLayout(vlayout1);
    addContent(frame);

    scrollArea = new QScrollArea();
    scrollArea->setObjectName("PropertyDialog-QScrollArea");
    QPalette palette = scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Background, Qt::NoBrush);
    scrollArea->viewport()->setPalette(palette);
    scrollArea->setFrameShape(QFrame::Shape::NoFrame);

    QFrame *infoframe = new QFrame;
    QVBoxLayout *scrollWidgetLayout = new QVBoxLayout;
    // 修复BUG-47113 UI显示不对
    scrollWidgetLayout->setContentsMargins(10, 0, 10, 20);
    scrollWidgetLayout->setSpacing(kArrowExpandSpacing);
    infoframe->setLayout(scrollWidgetLayout);

    scrollArea->setWidget(infoframe);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    QVBoxLayout *scrolllayout = new QVBoxLayout;
    scrolllayout->addWidget(scrollArea);
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(this->layout());
    layout->addLayout(scrolllayout, 1);

    deviceBasicWidget = new DeviceBasicWidget(this);

    setFixedWidth(350);
}

int DevicePropertyDialog::contentHeight() const
{
    int expandsHeight = kArrowExpandSpacing;
    for (const QWidget *expand : extendedControl) {
        expandsHeight += expand->height();
    }

#define DIALOG_TITLEBAR_HEIGHT 50
    return (DIALOG_TITLEBAR_HEIGHT
            + deviceIcon->height()
            + deviceName->height()
            + basicInfo->height()
            + devicesProgressBar->height()
            + expandsHeight
            + contentsMargins().top()
            + contentsMargins().bottom()
            + 40);
}

void DevicePropertyDialog::setSelectDeviceInfo(const DSC_NAMESPACE::Property::DeviceInfo &info)
{
    currentFileUrl = info.deviceUrl;
    deviceIcon->setPixmap(info.icon.pixmap(128, 128));
    deviceName->setText(info.deviceName);
    deviceBasicWidget->selectFileInfo(info);
    basicInfo->setLeftValue(info.deviceName, Qt::ElideNone, Qt::AlignLeft, true);
    setProgressBar(info.totalCapacity, info.availableSpace);
    addExtendedControl(deviceBasicWidget);
}

void DevicePropertyDialog::handleHeight(int height)
{
    QRect rect = geometry();
    rect.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rect);
}

void DevicePropertyDialog::setProgressBar(qint64 totalSize, qint64 freeSize)
{
    devicesProgressBar->setMaximum(10000);
    devicesProgressBar->setValue(totalSize && ~totalSize ? int(10000. * (totalSize - freeSize) / totalSize) : 0);
    QString sizeTotalStr = UniversalUtils::sizeFormat(totalSize, 1);
    QString sizeFreeStr = UniversalUtils::sizeFormat(totalSize - freeSize, 1);
    basicInfo->setRightValue(sizeFreeStr + QString("/") + sizeTotalStr, Qt::ElideNone, Qt::AlignRight, true);

    // 在浅色模式下，手动设置进度条背景色
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        DPalette palette = devicesProgressBar->palette();
        palette.setBrush(DPalette::ObviousBackground, QColor("#ededed"));
        DApplicationHelper::instance()->setPalette(devicesProgressBar, palette);
    }

    // 进度条背景色跟随主题变化而变化
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=](DGuiApplicationHelper::ColorType type) {
        DPalette palette = devicesProgressBar->palette();
        if (type == DGuiApplicationHelper::LightType) {
            palette.setBrush(DPalette::ObviousBackground, QColor("#ededed"));
            DApplicationHelper::instance()->setPalette(devicesProgressBar, palette);
        } else {
            palette.setBrush(DPalette::ObviousBackground, QColor("#4e4e4e"));
            DApplicationHelper::instance()->setPalette(devicesProgressBar, palette);
        }
    });
}

void DevicePropertyDialog::insertExtendedControl(int index, QWidget *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    vlayout->insertWidget(index, widget, 0, Qt::AlignTop);
    QMargins cm = vlayout->contentsMargins();
    QRect rc = contentsRect();
    widget->setFixedWidth(rc.width() - cm.left() - cm.right());
    extendedControl.append(widget);
    DEnhancedWidget *hanceedWidget = new DEnhancedWidget(widget, widget);
    connect(hanceedWidget, &DEnhancedWidget::heightChanged, this, &DevicePropertyDialog::handleHeight);
}

void DevicePropertyDialog::addExtendedControl(QWidget *widget)
{
    QVBoxLayout *vlayout = qobject_cast<QVBoxLayout *>(scrollArea->widget()->layout());
    insertExtendedControl(vlayout->count(), widget);
}

void DevicePropertyDialog::showEvent(QShowEvent *event)
{
    DDialog::showEvent(event);

    //展示时须设置弹窗尺寸，防止最小化后再次展示时窗口大小异常
    QRect rc = geometry();
    rc.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rc);
}

void DevicePropertyDialog::closeEvent(QCloseEvent *event)
{
    emit closed(currentFileUrl);
    DDialog::closeEvent(event);
}

void DevicePropertyDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
    DDialog::keyPressEvent(event);
}

DFMRoundBackground::DFMRoundBackground(QWidget *parent, int radius)
    : QObject(parent)
{
    parent->installEventFilter(this);
    setProperty("radius", radius);
}

DFMRoundBackground::~DFMRoundBackground()
{
    parent()->removeEventFilter(this);
}

bool DFMRoundBackground::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent() && event->type() == QEvent::Paint) {
        QWidget *w = dynamic_cast<QWidget *>(watched);
        if (!w) {
            return false;
        }
        int radius = property("radius").toInt();

        QPainter painter(w);
        QRectF bgRect;
        bgRect.setSize(w->size());
        const QPalette pal = QGuiApplication::palette();
        QColor bgColor = pal.color(QPalette::Base);

        QPainterPath path;
        path.addRoundedRect(bgRect, radius, radius);
        // drawbackground color
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path, bgColor);
        painter.setRenderHint(QPainter::Antialiasing, false);
        return true;
    }

    return QObject::eventFilter(watched, event);
}
