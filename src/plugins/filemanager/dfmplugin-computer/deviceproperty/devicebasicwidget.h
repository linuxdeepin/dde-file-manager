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
#ifndef DEVICEBASICWIDGET_H
#define DEVICEBASICWIDGET_H

#include "dfmplugin_computer_global.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "services/common/propertydialog/property_defines.h"
#include "dfm-base/utils/filestatisticsjob.h"

#include <DArrowLineDrawer>

DPCOMPUTER_BEGIN_NAMESPACE
class DeviceBasicWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit DeviceBasicWidget(QWidget *parent = nullptr);
    virtual ~DeviceBasicWidget() override;

private:
    void initUI();

public:
    void selectFileUrl(const QUrl &url);

    void selectFileInfo(const DSC_NAMESPACE::Property::DeviceInfo &info);

public slots:
    void slotFileDirSizeChange(qint64 size, int filesCount, int directoryCount);

signals:
    void heightChanged(int height);

private:
    DFMBASE_NAMESPACE::KeyValueLabel *deviceType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *deviceTotalSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileSystem { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCount { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *freeSize { nullptr };
    QFrame *deviceInfoFrame { nullptr };
    DFMBASE_NAMESPACE::FileStatisticsJob *fileCalculationUtils { nullptr };
};
DPCOMPUTER_END_NAMESPACE
#endif   // DEVICEBASICWIDGET_H
