// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICWIDGET_H
#define BASICWIDGET_H

#include "dfmplugin_vault_global.h"

#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/utils/filestatisticsjob.h>

#include <DArrowLineDrawer>

#include <QCheckBox>

namespace dfmplugin_vault {
class BasicWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit BasicWidget(QWidget *parent = nullptr);
    virtual ~BasicWidget() override;

private:
    void initUI();

public:
    void selectFileUrl(const QUrl &url);

    qint64 getFileSize();

    int getFileCount();

public slots:

    void slotFileCountAndSizeChange(qint64 size, int filesCount, int directoryCount);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    DFMBASE_NAMESPACE::KeyValueLabel *fileSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCount { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *filePosition { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCreated { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileModified { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileAccessed { nullptr };
    DFMBASE_NAMESPACE::FileStatisticsJob *fileCalculationUtils { nullptr };
    int fSize { 0 };
    int fCount { 0 };
};
}
#endif   // BASICWIDGET_H
