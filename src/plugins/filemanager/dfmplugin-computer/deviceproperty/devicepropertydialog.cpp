// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicepropertydialog.h"
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/elidetextlayout.h>

#include <DDrawer>
#include <denhancedwidget.h>
#include <DArrowLineDrawer>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>

#include <QKeyEvent>
#include <QPainterPath>

const static int kArrowExpandSpacing { 10 };
const static int kForecastDisplayHeight { 610 };

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace dfmplugin_computer;
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

    deviceNameLayout = new QVBoxLayout(this);
    deviceNameLayout->setContentsMargins(0, 0, 0, 0);

    QFrame *basicInfoFrame = new QFrame(this);

    basicInfo = new KeyValueLabel(this);
    basicInfo->setLeftFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::DemiBold);
    basicInfo->setLeftValueLabelFixedWidth(150);

    devicesProgressBar = new DColoredProgressBar();
    devicesProgressBar->addThreshold(0, QColor(0xFF0080FF));
    devicesProgressBar->addThreshold(7000, QColor(0xFFFFAE00));
    devicesProgressBar->addThreshold(9000, QColor(0xFFFF0000));
    devicesProgressBar->setMaximumHeight(8);
    devicesProgressBar->setTextVisible(false);

    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(12, 8, 12, 8);
    vlayout->addWidget(basicInfo);
    vlayout->addWidget(devicesProgressBar);
    basicInfoFrame->setLayout(vlayout);

    new DFMRoundBackground(basicInfoFrame, 8);

    QVBoxLayout *vlayout1 = new QVBoxLayout();
    vlayout1->setContentsMargins(0, 0, 0, 0);
    vlayout1->setSpacing(0);
    vlayout1->addWidget(deviceIcon, 0, Qt::AlignHCenter | Qt::AlignTop);
    vlayout1->addLayout(deviceNameLayout);
    vlayout1->addWidget(basicInfoFrame);

    QFrame *frame = new QFrame(this);
    frame->setLayout(vlayout1);
    addContent(frame);

    scrollArea = new QScrollArea();
    scrollArea->setObjectName("PropertyDialog-QScrollArea");
    QPalette palette = scrollArea->viewport()->palette();
    palette.setBrush(QPalette::Window, Qt::NoBrush);
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

    setProperty("ForecastDisplayHeight", QVariant::fromValue(kForecastDisplayHeight));
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
            + deviceNameFrame->height()
            + basicInfo->height()
            + devicesProgressBar->height()
            + expandsHeight
            + contentsMargins().top()
            + contentsMargins().bottom()
            + 40);
}

void DevicePropertyDialog::setSelectDeviceInfo(const DeviceInfo &info)
{
    currentFileUrl = info.deviceUrl;
    deviceIcon->setPixmap(info.icon.pixmap(128, 128));
    setFileName(info.deviceName);
    deviceBasicWidget->selectFileInfo(info);
    QString deviceShowName = info.deviceDesc.isEmpty() ? info.deviceName : QString("%1(%2)").arg(info.deviceName).arg(info.deviceDesc);
    basicInfo->setLeftValue(deviceShowName, Qt::ElideMiddle, Qt::AlignLeft, true);
    setProgressBar(info.totalCapacity, info.availableSpace, !info.mountPoint.isEmpty());
    addExtendedControl(deviceBasicWidget);
}

void DevicePropertyDialog::handleHeight(int height)
{
    QRect rect = geometry();
    rect.setHeight(contentHeight() + kArrowExpandSpacing * 2);
    setGeometry(rect);
}

void DevicePropertyDialog::setProgressBar(qint64 totalSize, qint64 freeSize, bool mounted)
{
    devicesProgressBar->setMaximum(10000);
    if (!mounted)
        freeSize = totalSize;
    devicesProgressBar->setValue(totalSize && ~totalSize ? int(10000. * (totalSize - freeSize) / totalSize) : 0);
    QString sizeTotalStr = UniversalUtils::sizeFormat(totalSize, 1);
    QString sizeFreeStr = UniversalUtils::sizeFormat(totalSize - freeSize, 1);
    if (mounted)
        basicInfo->setRightValue(sizeFreeStr + QString("/") + sizeTotalStr, Qt::ElideNone, Qt::AlignRight, true);
    else
        basicInfo->setRightValue(sizeTotalStr, Qt::ElideNone, Qt::AlignRight, true);
    basicInfo->setRightFontSizeWeight(DFontSizeManager::SizeType::T7, QFont::DemiBold);

    // 在浅色模式下，手动设置进度条背景色
    if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
        DPalette palette = devicesProgressBar->palette();
        palette.setBrush(DPalette::ObviousBackground, QColor("#ededed"));
        DPaletteHelper::instance()->setPalette(devicesProgressBar, palette);
    }

    // 进度条背景色跟随主题变化而变化
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [=](DGuiApplicationHelper::ColorType type) {
        DPalette palette = devicesProgressBar->palette();
        if (type == DGuiApplicationHelper::LightType) {
            palette.setBrush(DPalette::ObviousBackground, QColor("#ededed"));
            DPaletteHelper::instance()->setPalette(devicesProgressBar, palette);
        } else {
            palette.setBrush(DPalette::ObviousBackground, QColor("#4e4e4e"));
            DPaletteHelper::instance()->setPalette(devicesProgressBar, palette);
        }
    });
}

void DevicePropertyDialog::setFileName(const QString &filename)
{
    if (deviceNameFrame)
        delete deviceNameFrame;

    deviceNameFrame = new QFrame(this);

    QRect rect(QPoint(0, 0), QSize(200, 66));
    QStringList labelTexts;
    ElideTextLayout layout(filename);
    layout.layout(rect, Qt::ElideMiddle, nullptr, Qt::NoBrush, &labelTexts);

    int textHeight = 0;
    QVBoxLayout *nameLayout = new QVBoxLayout;
    for (const auto &labelText : labelTexts) {
        DLabel *fileNameLabel = new DLabel(labelText, deviceNameFrame);
        fileNameLabel->setAlignment(Qt::AlignHCenter);
        textHeight += fileNameLabel->fontInfo().pixelSize() + 10;
        nameLayout->addWidget(fileNameLabel, 0, Qt::AlignHCenter);

        if (fileNameLabel->fontMetrics().horizontalAdvance(labelText) > (rect.width() - 10))
            fileNameLabel->setFixedWidth(rect.width());
    }

    nameLayout->setContentsMargins(0, 0, 0, 0);
    nameLayout->setSpacing(0);
    deviceNameFrame->setLayout(nameLayout);
    nameLayout->addStretch(1);
    deviceNameFrame->setFixedHeight(textHeight + 15);
    deviceNameLayout->addWidget(deviceNameFrame);
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
