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

#ifndef DEVICEPROPERTYVIEW_H
#define DEVICEPROPERTYVIEW_H

#include "dfmplugin_propertydialog_global.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "devicebasicwidget.h"
#include "services/common/propertydialog/property_defines.h"

#include <DDialog>
#include <DColoredProgressBar>

#include <QScrollArea>

const int EXTEND_FRAME_MAXHEIGHT = 160;

DWIDGET_BEGIN_NAMESPACE
class DArrowLineDrawer;
DWIDGET_END_NAMESPACE

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class DevicePropertyDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DevicePropertyDialog(QWidget *parent = nullptr);
    virtual ~DevicePropertyDialog() override;

private:
    void iniUI();

    int contentHeight() const;

    void handleHeight(int height);

    void setProgressBar(qint64 totalSize, qint64 freeSize);

public slots:
    void setSelectDeviceInfo(const DSC_NAMESPACE::DeviceInfo &info);

    void insertExtendedControl(int index, QWidget *widget);

    void addExtendedControl(QWidget *widget);

    //    void insertExtendedControl(int index, DFMBASE_NAMESPACE::ExtendedControlDrawerView *widget, bool expansion = false);

    //    void addExtendedControl(DFMBASE_NAMESPACE::ExtendedControlDrawerView *widget, bool expansion = false);

signals:
    void closed(const QUrl &url);

protected:
    void showEvent(QShowEvent *event) override;

    void closeEvent(QCloseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private:
    DTK_WIDGET_NAMESPACE::DLabel *deviceName { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *deviceIcon { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *basicInfo { nullptr };
    DTK_WIDGET_NAMESPACE::DColoredProgressBar *devicesProgressBar { nullptr };
    DeviceBasicWidget *deviceBasicWidget { nullptr };
    QScrollArea *scrollArea { nullptr };
    QList<QWidget *> extendedControl {};
    QUrl currentFileUrl {};
};
DPPROPERTYDIALOG_END_NAMESPACE
#endif   // DEVICEPROPERTYVIEW_H
