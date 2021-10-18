/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "diskcontrolwidget.h"

#include "dbus_interface/devicemanagerdbus_interface.h"

#include <DGuiApplicationHelper>

#include <QScrollBar>
#include <QLabel>

static const int WIDTH = 300;

/*!
 * \class DiskControlWidget
 *
 * \brief DiskControlWidget is DiskMountPlugin::itemPopupApplet
 * The control pops up after the left mouse button click on the main control of the plugin
 */

DiskControlWidget::DiskControlWidget(QWeakPointer<DeviceManagerInterface> inter, QWidget *parent)
    : QScrollArea(parent),
      centralLayout(new QVBoxLayout),
      centralWidget(new QWidget),
      deviceInter(inter)
{
    this->setObjectName("DiskControlWidget-QScrollArea");
    initializeUi();
    initConnection();
}

void DiskControlWidget::initializeUi()
{
    std::call_once(DiskControlWidget::onceFlag(), [this] () {
        centralWidget->setLayout(centralLayout);
        centralWidget->setFixedWidth(WIDTH);
        centralLayout->setMargin(0);
        centralLayout->setSpacing(0);
        setWidget(centralWidget);
        setFixedWidth(WIDTH);
        setFrameShape(QFrame::NoFrame);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        verticalScrollBar()->setSingleStep(7);
        centralWidget->setAutoFillBackground(false);
        viewport()->setAutoFillBackground(false);
        paintUi();
    });
}

void DiskControlWidget::initConnection()
{
    // When the system theme changes,
    // refreshes the list of controls to fit the text color under the new theme
    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::themeTypeChanged,
            this, &DiskControlWidget::onDiskListChanged);
}

void DiskControlWidget::removeWidgets()
{
    while (QLayoutItem *item = centralLayout->takeAt(0)) {
        delete item->widget();
        delete item;
    }
}

void DiskControlWidget::paintUi()
{
    QVBoxLayout *headerLay = new QVBoxLayout(this);
    QWidget *header = new QWidget(this);
    header->setLayout(headerLay);
    headerLay->setSpacing(0);
    headerLay->setContentsMargins(20, 9, 0, 8);
    QLabel *headerTitle = new QLabel(tr("Disks"), this);
    QFont f = headerTitle->font();
    f.setPixelSize(20);
    f.setWeight(QFont::Medium);
    headerTitle->setFont(f);
    QPalette pal = headerTitle->palette();
    QColor color = Dtk::Gui::DGuiApplicationHelper::instance()->themeType() == Dtk::Gui::DGuiApplicationHelper::LightType
            ? Qt::black
            : Qt::white;
    pal.setColor(QPalette::WindowText, color);
    headerTitle->setPalette(pal);

    headerLay->addWidget(headerTitle);
    centralLayout->addWidget(header);

    addSeparateLineUi(2);
    int mountedCount = addDeviceItems();
    emit diskCountChanged(mountedCount);

    const int contentHeight = mountedCount * 70 + 46;
    const int maxHeight = std::min(contentHeight, 70 * 6);
    centralWidget->setFixedHeight(contentHeight);
    setFixedHeight(maxHeight);

    verticalScrollBar()->setPageStep(maxHeight);
    verticalScrollBar()->setMaximum(contentHeight - maxHeight);
}

void DiskControlWidget::addSeparateLineUi(int width)
{
    QFrame *line = new QFrame(this);
    line->setLineWidth(width);
    line->setFrameStyle(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    centralLayout->addWidget(line);
}

int DiskControlWidget::addDeviceItems()
{
    int mountedCount = 0;

    // TODO(zhangs): request server, remove last seperate line

    return mountedCount;
}

std::once_flag &DiskControlWidget::onceFlag()
{
    static std::once_flag flag;
    return flag;
}

void DiskControlWidget::onDiskListChanged()
{
    removeWidgets();
    paintUi();
}



