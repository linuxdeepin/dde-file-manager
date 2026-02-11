// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEBASICWIDGET_H
#define DEVICEBASICWIDGET_H

#include "dfmplugin_computer_global.h"
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/filestatisticsjob.h>

#include <DArrowLineDrawer>

namespace dfmplugin_computer {
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

    void selectFileInfo(const DeviceInfo &info);

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
}
#endif   // DEVICEBASICWIDGET_H
